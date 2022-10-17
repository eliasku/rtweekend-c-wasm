#ifndef RT_MATH_H
#define RT_MATH_H

#include "common.h"

#define INFINITY 10000000.0f
#define PI 3.1415926535897932385

inline float degrees_to_radians(float degrees) {
    return degrees * (float)(PI / 180.0);
}

inline float saturate(float x) {
    return x > 1.0f ? 1.0f : (x < 0.0f ? 0.0f : x);
}

// vec3

typedef float vec3 __attribute__((ext_vector_type(3)));

vec3 zero_vec3() {
    return (vec3){0, 0, 0};
}

vec3 splat_vec3(float s) {
    return (vec3){s, s, s};
}

#define VEC3(x,y,z) ((vec3){(x),(y),(z)})

float dot_vec3(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float length_squared_vec3(vec3 a) {
    return dot_vec3(a, a);
}

bool almost_zero_vec3(vec3 v) {
    const float eps = 1e-5f;
    return FABS(v.x) < eps && FABS(v.y) < eps && FABS(v.z) < eps;
}

float length_vec3(vec3 a) {
    return SQRTF(length_squared_vec3(a));
}

vec3 cross_vec3(vec3 a, vec3 b) {
     return (vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

vec3 normalize_vec3(vec3 a) {
    return a / length_vec3(a);
}

vec3 reflect_vec3(vec3 v, vec3 n) {
    return v - 2 * dot_vec3(v, n) * n;
}

vec3 refract_vec3(vec3 uv, vec3 n, float etai_over_etat) {
    float cos_theta = dot_vec3(-uv, n);
    if(cos_theta > 1) cos_theta = 1.0f;
    vec3 r_out_perp =  etai_over_etat * (uv + cos_theta * n);
    vec3 r_out_parallel = -SQRTF(FABS(1.0f - length_squared_vec3(r_out_perp))) * n;
    return r_out_perp + r_out_parallel;
}

// RANDOM

#define TEMPERING_MASK_B    0x9D2C5680
#define TEMPERING_MASK_C    0xEFC60000

static uint32_t ek_temper(uint32_t x) {
    x ^= x >> 11;
    x ^= (x << 7) & TEMPERING_MASK_B;
    x ^= (x << 15) & TEMPERING_MASK_C;
    x ^= x >> 18;
    return x;
}

static uint32_t rand_state = 111;

// simple PRNG from libc with u32 state
uint32_t ek_rand1(uint32_t* state) {
    uint32_t x = *state;
    x = x * 1103515245 + 12345;
    *state = x;
    return ek_temper(x) >> 1;
}

float unorm_f32_from_u32(uint32_t value) {
    const uint32_t exponent = 127;
    const uint32_t mantissa = value & ((1u << 23u) - 1u);
    union {
        uint32_t u32;
        float f32;
    } bits;
    bits.u32 = (exponent << 23u) | mantissa;
    return bits.f32 - 1.0f;
}

float rnd_f(uint32_t* state) {
    return unorm_f32_from_u32(ek_rand1(state));
}

vec3 random_unit_vector(void) {
    while (true) {
        vec3 p = (vec3){
            rnd_f(&rand_state) * 2 - 1,
            rnd_f(&rand_state) * 2 - 1,
            rnd_f(&rand_state) * 2 - 1
        };
        float sd = length_squared_vec3(p);
        if (sd > 1) continue;
        return p / SQRTF(sd);
    }
}

// ray

typedef struct ray {
    vec3 origin;
    vec3 dir;
} ray;

vec3 at_ray(ray ray, float t) {
    return ray.origin + t * ray.dir;
}

typedef struct material {
    int type;
    vec3 albedo;
    float fuzz;
    float ir;
} material;

typedef struct hit_record {
    vec3 p;
    vec3 n;
    float t;
    material* mat;
    bool front_face;
} hit_record;

typedef struct sphere {
    vec3 center;
    float radius;
    material* mat;
} sphere;

bool scatter_lambertian(material* mat, ray in, hit_record* hit, vec3* attenuation, ray* scattered) {
    scattered->origin = hit->p;
    scattered->dir = hit->n + random_unit_vector();
    if(almost_zero_vec3(scattered->dir)) {
        scattered->dir = hit->n;
    }
    *attenuation = mat->albedo;
    return true;
}

bool scatter_metal(material* mat, ray in, hit_record* hit, vec3* attenuation, ray* scattered) {
    vec3 reflected = reflect_vec3(normalize_vec3(in.dir), hit->n);
    scattered->origin = hit->p;
    scattered->dir = reflected + mat->fuzz * random_unit_vector();
    *attenuation = mat->albedo;
    return dot_vec3(scattered->dir, hit->n) > 0;
}

float reflectance(float cosine, float ref_idx) {
    // Use Schlick's approximation for reflectance.
    float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
    r0 = r0 * r0;
    float s = 1.0f - cosine;
    float s2 = s * s;
    // s ^ 5
    s *= s2 * s2;
    return r0 + (1.0f - r0) * s;
}

bool scatter_dielectric(material* mat, ray in, hit_record* hit, vec3* attenuation, ray* scattered) {
    float refraction_ratio = hit->front_face ? (1.0f / mat->ir) : mat->ir;

    vec3 unit_direction = normalize_vec3(in.dir);
    float cos_theta = FMIN(dot_vec3(-unit_direction, hit->n), 1.0f);
    float sin_theta = SQRTF(1.0f - cos_theta * cos_theta);
    bool cannot_refract = refraction_ratio * sin_theta > 1.0f;
    vec3 direction;
    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > rnd_f(&rand_state))
        direction = reflect_vec3(unit_direction, hit->n);
    else
        direction = refract_vec3(unit_direction, hit->n, refraction_ratio);

    *attenuation = splat_vec3(1);
    scattered->origin = hit->p;
    scattered->dir = direction;
    return true;
}

bool scatter(material* mat, ray in, hit_record* hit, vec3* attenuation, ray* scattered) {
    if(mat->type == 0) {
        return scatter_lambertian(mat, in, hit, attenuation, scattered);
    }
    else if(mat->type == 1) {
        return scatter_metal(mat, in, hit, attenuation, scattered);
    }
    else {
        return scatter_dielectric(mat, in, hit, attenuation, scattered);
    }
}


void set_hit_normal(hit_record* hit, const ray r, const vec3 outward_normal) {
    hit->front_face = dot_vec3(r.dir, outward_normal) < 0;
    hit->n = hit->front_face ? outward_normal : -outward_normal;
}

bool hit_sphere(const sphere s, const ray r, float t_min, float t_max, hit_record* hit) {
    vec3 oc = r.origin - s.center;
    float a = length_squared_vec3(r.dir);
    float half_b = dot_vec3(oc, r.dir);
    float c = length_squared_vec3(oc) - s.radius * s.radius;
    float discriminant = half_b * half_b - a * c;
    if(discriminant < 0.0f) {
        return false;
    }
    discriminant = SQRTF(discriminant);
    // Find the nearest root that lies in the acceptable range.
    float root = (-half_b - discriminant) / a;
    if (root < t_min || t_max < root) {
        root = (-half_b + discriminant) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    hit->t = root;
    hit->p = at_ray(r, root);
    const vec3 outward_normal = (hit->p - s.center) / s.radius;
    set_hit_normal(hit, r, outward_normal);
    hit->mat = s.mat;
    return true;
}

bool hit_list(const sphere* s, int n, ray r, float t_min, float t_max, hit_record* rec) {
    hit_record temp_rec = {0};
    bool hit_anything = false;
    float closest_so_far = t_max;
    for (int i = 0; i < n; ++i) {
        if (hit_sphere(s[i], r, t_min, closest_so_far, &temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            *rec = temp_rec;
        }
    }
    return hit_anything;
}

// camera
typedef struct camera {
    vec3 origin;
    vec3 lower_left_corner;
    vec3 horizontal;
    vec3 vertical;
    vec3 u;
    vec3 v;
    vec3 w;
    float lens_radius;
} camera;

camera new_camera(
        vec3 lookfrom,
        vec3 lookat,
        vec3 vup,
        float vfov,  // vertical field-of-view in degrees
        float aspect_ratio,
        float aperture,
        float focus_dist
) {
    float theta = degrees_to_radians(vfov);
    float h = TANF(theta / 2);
    float viewport_height = 2.0f * h;
    float viewport_width = aspect_ratio * viewport_height;

    vec3 w = normalize_vec3(lookfrom - lookat);
    vec3 u = normalize_vec3(cross_vec3(vup, w));
    vec3 v = cross_vec3(w, u);

    const vec3 origin = lookfrom;
    const vec3 horizontal = focus_dist * viewport_width * u;;
    const vec3 vertical = focus_dist * viewport_height * v;
    const vec3 lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

    return (camera) {
        .origin = origin,
        .lower_left_corner = lower_left_corner,
        .horizontal = horizontal,
        .vertical = vertical,
        .u = u,
        .v = v,
        .w = w,
        .lens_radius = aperture / 2
    };
}

vec3 random_in_unit_disk(void) {
    float a = rnd_f(&rand_state);
    float d = SQRTF(rnd_f(&rand_state));
    return VEC3(d * COSF(a), d * SINF(a), 0);
}

ray get_camera_ray(const camera* c, float s, float t) {
    vec3 rd = c->lens_radius * random_in_unit_disk();
    vec3 offset = c->u * rd.x + c->v * rd.y;
    return (ray){
        .origin = c->origin + offset,
        .dir = c->lower_left_corner + s * c->horizontal + t * c->vertical - c->origin - offset
    };
}

#endif // RT_MATH_H
