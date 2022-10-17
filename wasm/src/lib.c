#include "imports.h"
#include "rt_math.h"

EXPORT void initialize(void) {
}

EXPORT void update(double ts) {
}

typedef struct world {
    sphere objects[512];
    material materials[512];
    int objects_num;
    int materials_num;
} world;

vec3 ray_color(ray r, const world* w, int depth) {
    hit_record hit = {0};
//    if (hit_list(w->objects, w->objects_num, r, 0.001f, INFINITY, &hit)) {
//        vec3 color = zero_vec3();
//        const int samples = 1;
//        if (depth > 1) {
//            for (int i = 0; i < samples; ++i) {
//                ray scattered;
//                vec3 attenuation;
//                if (scatter(hit.mat, r, &hit, &attenuation, &scattered)) {
//                    color += attenuation * ray_color(scattered, w, depth - 1);
//                }
//            }
//        }
//        return color / samples;
//    }
    ray scattered;
    vec3 attenuation;
    if (depth > 1 &&
        hit_list(w->objects, w->objects_num, r, 0.001f, INFINITY, &hit) &&
        scatter(hit.mat, r, &hit, &attenuation, &scattered)) {
        return attenuation * ray_color(scattered, w, depth - 1);
    }
    const vec3 unit_direction = normalize_vec3(r.dir);
    float t = 0.5f * (unit_direction.y + 1.0f);
    return (1.0f - t) * VEC3(1, 1, 1) + t * VEC3(0.5f, 0.7f, 1.0f);
}

EXPORT uint32_t create_world(world* w) {
    w->materials[0] = (material) {0, (vec3) {0.5f, 0.5f, 0.5f}};
    w->materials[1] = (material) {2, (vec3) {0.5f, 0.5f, 0.5f}, 0, 1.5f};
    w->materials[2] = (material) {0, (vec3) {0.4f, 0.2f, 0.1f}};
    w->materials[3] = (material) {1, (vec3) {0.7f, 0.6f, 0.5f}, 0};
    w->objects[0] = (sphere) {(vec3) {0, -1000, 0}, 1000, &w->materials[0]};
    w->objects[1] = (sphere) {(vec3) {0, 1, 0}, 1, &w->materials[1]};
    w->objects[2] = (sphere) {(vec3) {-4, 1, 0}, 1, &w->materials[2]};
    w->objects[3] = (sphere) {(vec3) {4, 1, 0}, 1, &w->materials[3]};

    int mat_n = 4;
    int obj_n = 4;

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            if (obj_n >= 512) { continue; }
            float choose_mat = rnd_f(&rand_state);
            vec3 center = VEC3((float)a + 0.9f * rnd_f(&rand_state), 0.2f, (float)b + 0.9f * rnd_f(&rand_state));

            if (length_squared_vec3(center - VEC3(4, 0.2f, 0)) > 0.9 * 0.9) {
                material* sphere_material = &w->materials[mat_n++];

                if (choose_mat < 0.8) {
                    // diffuse
                    const vec3 albedo = VEC3(rnd_f(&rand_state), rnd_f(&rand_state), rnd_f(&rand_state));
                    sphere_material->type = 0;
                    sphere_material->albedo = albedo * albedo;

                } else if (choose_mat < 0.95) {
                    // metal
                    sphere_material->type = 1;
                    sphere_material->albedo = VEC3(0.5f + 0.5f * rnd_f(&rand_state),
                                                       0.5f + 0.5f * rnd_f(&rand_state),
                                                       0.5f + 0.5f * rnd_f(&rand_state));
                    sphere_material->fuzz = 0.5f * rnd_f(&rand_state);
                } else {
                    // glass
                    sphere_material->ir = 1.5f;
                }
                w->objects[obj_n++] = (sphere) {center, 0.2f, sphere_material};
            }
        }
    }
    w->materials_num = mat_n;
    w->objects_num = obj_n;
    return sizeof(world);
}

EXPORT void render(world* w, int width, int height, int n, float* colors, uint8_t* pixels) {
    const camera cam = new_camera(
            VEC3(13, 2, 3),
            VEC3(0, 0, 0),
            VEC3(0, 1, 0),
            20,
            (float) width / (float)height,
            0.1f,
            10
    );
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            vec3 pixel_color = VEC3(colors[0], colors[1], colors[2]);
            const float du = rnd_f(&rand_state);
            const float dv = rnd_f(&rand_state);
            const float u = ((float) i + du) / (float) (width - 1);
            const float v = ((float) j + dv) / (float) (height - 1);
            const ray r = get_camera_ray(&cam, u, 1.0f - v);
            pixel_color += ray_color(r, w, 6);
            colors[0] = pixel_color.x;
            colors[1] = pixel_color.y;
            colors[2] = pixel_color.z;
            colors += 3;
            pixel_color.x = SQRTF(saturate(pixel_color.x / n));
            pixel_color.y = SQRTF(saturate(pixel_color.y / n));
            pixel_color.z = SQRTF(saturate(pixel_color.z / n));
            *(pixels++) = (uint8_t) (pixel_color.x * 255.0f);
            *(pixels++) = (uint8_t) (pixel_color.y * 255.0f);
            *(pixels++) = (uint8_t) (pixel_color.z * 255.0f);
            *(pixels++) = 0xFF;
        }
    }
}
