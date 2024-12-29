#include <cstdlib>
#include <cstring>
#include <cstdint>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS
#define __STDC_LIB_EXT1__
#include "png.h"

#include <iostream>
#include <iomanip>

struct Vec3
{
    float x, y, z;
    
};

Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 operator+(const Vec3& a, const Vec3& b)
{
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 operator*(const Vec3& a, float s)
{
	return { a.x * s, a.y * s, a.z * s };
}

Vec3 operator*(float s, const Vec3& a)
{
	return { a.x * s, a.y * s, a.z * s };
}

float dot(const Vec3& a, const Vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float mag(const Vec3& a)
{
    return sqrtf(dot(a, a));
}

float saturate(float a)
{
	if (a < 0.0f)
		return 0.0f;

    if (a > 1.0f)
        return 1.0f;

    return a;
}

Vec3 norm(const Vec3& a)
{
    // multiplication by the inverse is the same as division
    return a * (1.f / mag(a));
}

Vec3 cross(const Vec3& a, const Vec3& b)
{
    return { a.y * b.z - a.z * b.y, a.x * b.z - a.z * b.x, a.x * b.y - a.y * b.x };
}


struct Ray
{
    Vec3 pos; // where ray starts
    Vec3 dir;
};


struct Sphere
{
    float radius;
    Vec3 pos;
};


struct Hit
{
    Vec3 pos; // position of a hit, where the ray hit the object
    Vec3 normal; // vector perpendicular to the surface at the point of intersection
    float distance; // distance along ray to the hit position
};

bool intersect(const Ray& ray, const Sphere& sphere, Hit& out_hit)
{
    Vec3 between = sphere.pos - ray.pos;

    // ray vector projected on to the vector between the sphere and the ray
    Vec3 projected = dot(between, ray.dir) * ray.dir;
    float d = mag(cross(ray.dir, between));
    //float d = sqrt(mag(between) * mag(between) - mag(projected) * mag(projected));
    //std::cout << std::setprecision(20) << std::fixed << "cross: " << d << ", dot: " << x << std::endl;

    // Hit
	if (d <= sphere.radius)
    {
        float t1 = dot(ray.dir, between);
        float t2 = sqrt(sphere.radius * sphere.radius - d * d);

        out_hit.distance = t1 - t2;
        // the end of the vector between start of the ray and the hit position
        out_hit.pos = ray.pos + ray.dir * out_hit.distance;
        // vector from the sphere center to the hit position
        out_hit.normal = norm(sphere.pos - out_hit.pos);

        // if the ray is perfectly in opposite direction hit will still be detected so we have to check 
        // if the ray and the sphere are in the same directions
        //if (dot(ray.dir, out_hit.normal) > 0.0f)
        //{
        //    out_hit.normal = -1.0f * out_hit.normal;
        //}

		return true;
    }
	return false;

    
}


Vec3 render(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    Vec3 camera_pos = { 0.f, 0.f, -8.f };
    float camera_near = 2.f;

    // pixel position on the camera near plane
    float aspect_ratio = width / (float)height;
    Vec3 pixel_pos = {
        aspect_ratio * (float)x / (float)width - (aspect_ratio - 1.f) * 0.5f - 0.5f,
        (float)y / (float)height - 0.5f,
        camera_pos.z + camera_near
    };

    Ray ray;
    ray.pos = pixel_pos;
    // point - point = vector between them
    ray.dir = norm(pixel_pos - camera_pos);

    Sphere sphere;
    sphere.pos = { 0.f, 0.f, 0.f };
    sphere.radius = 0.5f;

    Hit hit;
    if (intersect(ray, sphere, hit))
    {
        // hit.normal is in range <-1, 1> but we need <0, 1>
        return (hit.normal + Vec3{1.0f, 1.0f, 1.0f}) * 0.5f;
    }


    Vec3 white = { 1.0f, 1.0f, 1.0f };
    Vec3 blue = { 0.4f, 0.7f, 1.0f };

    // linear interpolation between white and blue
    // we don't want t to be more than 1 or less than 0, so we have to saturate it!
    float t = saturate(0.5f * (ray.dir.y + 1.0f));

    return white * t + blue * (1.0f - t);
}

int main()
{
    const uint32_t width = 1024;
    const uint32_t height = 768;

    const uint32_t stride = 3;
    const uint32_t image_size = width * height * stride;

    void* image = malloc(image_size);
    memset(image, 0xFF, image_size);

    // pixel is 1 byte so it's max is 255 and the rgb max is also 255
    // we will move by 3 (rgb = 3 values in range 0-255) every loop iteration
    uint8_t* pixel = (uint8_t*)image;

	for (uint32_t y = 0; y < height; ++y)
    {
		for (uint32_t x = 0; x < width; ++x)
        {
            Vec3 color = render(x, y, width, height);

			// Assign the pixel colors
			pixel[0] = color.x * 255.f;
			pixel[1] = color.y * 255.f;
			pixel[2] = color.z * 255.f;

			pixel += stride;
        }
    }

    int32_t res = stbi_write_png("render.png", width, height, 3, image, width*stride);

    if (res)
    {
        printf("Saved to render.png\n");
		std::system("start render.png");
    }
    else
        printf("Cannot save to render.png\n");

    //std::cin.get();
    return 0;
}