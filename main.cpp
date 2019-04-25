//Zhengpeng Li, W1264508, COEN290 HW3
//  main.cpp
//  A3
//
//  Created by zhengpeng li on 11/24/17.
//  Copyright © 2017 zhengpeng li. All rights reserved.
//

#include <iostream>
#include <GLUT/glut.h>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <limits>

const unsigned int W = 800;
const unsigned int H = 600;

// A generic 3D vector with overload operator to make computation easier.
// (Using struct as everything should be public. Same for Sphere below.)
struct Vector {
    // Minimalistic constructor.
    Vector(float xx, float yy, float zz)
        : x(xx), y(yy), z(zz) {}
    
    Vector() {}
  
    // Length.
    float length() const {
        return sqrt(x*x + y*y + z*z);
    }
    
    float length2() const {
        return x*x + y*y + z*z;
    }
    
    // Set to unit vector.
    Vector& normalize() {
        float length = this->length();
        if (length > 0) {
            x /= length;
            y /= length;
            z /= length;
        }
        return *this;
    }
    
    // Dot product.
    float dot(const Vector& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
    // Overload some common operators.
    Vector operator+ (const Vector &v) const {
        return Vector(x + v.x, y + v.y, z + v.z);
    }

    Vector operator- (const Vector &v) const {
        return Vector(x - v.x, y - v.y, z - v.z);
    }
    
    Vector operator* (float d) const {
        return Vector(x * d, y * d, z * d);
    }

    Vector operator* (const Vector &v) const {
        return Vector(x * v.x, y * v.y, z * v.z);
    }
    
    float x = 0;
    float y = 0;
    float z = 0;
};
// 3D point is using the same functions.
typedef Vector Point;
// Color is also using the same functions.
typedef Vector RGB_float;

// Contains sphere specification and helper methods.
struct Sphere {
    Sphere(Point c, float r, RGB_float cl)
        : center(c), radius(r), color(cl) {}
    Sphere() {}
    
    // Check if there is an intersection for a ray coming from Point o.
    // Set distance to the nearest intersection distance if so.
    bool calculateIntersect(const Point& o, const Vector& ray, float* distance) {
        Vector l = o - center;
        float b = 2 * l.dot(ray);
        float c = l.length2() - radius * radius;
        float det = b * b - 4 * c;
        
        // No intersection.
        if (det < 0) return false;
        // With intersection, set the closer distance
        *distance = (-1 * b - sqrt(det))/2;
        return true;
    }
    
    // Calculate the color at a point on sphere.
    RGB_float colorAt(const Point& point, const Point& light, const RGB_float& lightColor) {
        float cosA = (point - center).normalize().dot((light - point).normalize());
        return color * lightColor * cosA;
    }
    
    // Center and radius of the sphere.
    Point center;
    float radius;
    RGB_float color;
};

// Instead of a single point camera, we render as if receiving light mostly in parallel.
// In this case, looking at the z-direction.
Vector cameraDir(0, 0, 1);
// Do a single ball first, generalize later.
std::vector<Sphere> s;
// The light source, consists of the location and the color.
Point light(0, 300, -100);
RGB_float lightColor(0.5, 1, 1);

// Main function that calls various helpers to determine color at pixel i, j.
RGB_float rayCast(int i, int j) {
    float xx = i + 0.5 - W/2;
    float yy = j + 0.5 - H/2;

    Point shiftEye(xx, yy, 0);
    
    float tmpDistance = 0;
    Point intersect;
    int nearestSphereInd = -1;
    float minDistance = std::numeric_limits<float>::infinity();
    for (int i = 0; i < s.size(); i++) {
        if (s[i].calculateIntersect(shiftEye, cameraDir, &tmpDistance)) {
            if (tmpDistance < minDistance) {
                minDistance = tmpDistance;
                nearestSphereInd = i;
            }
        }
    }
    // When the ray intersect with at least one sphere.
    if (nearestSphereInd != -1) {
        Point intersect = shiftEye + cameraDir * minDistance;
        // Checks if there is another sphere blocking the intersect from light source.
        int nShadowed = 0;
        for (int i = 0; i < s.size(); i++) {
            if (i == nearestSphereInd) continue;
            Vector shadowRay = light - intersect;
            float distanceToLight = shadowRay.length();
            shadowRay.normalize();
            if (s[i].calculateIntersect(intersect, shadowRay, &tmpDistance)) {
                if (tmpDistance > 0 && tmpDistance < distanceToLight) {  // Blocked
                    nShadowed++;
                }
            }
        }

        // shadow simply blocks light by a percentage for each sphere instead of rendering totally black.
        return s[nearestSphereInd].colorAt(intersect, light, lightColor) * pow(0.5, nShadowed);
    }
    
    // Default to grey.
    return RGB_float(0.1, 0.1, 0.1);
}

// Testing function. To be removed.
RGB_float randomColor() {
    RGB_float res;
    res.x = static_cast<float>( rand() % 256 ) / 256.0f;
    res.y = static_cast<float>( rand() % 256 ) / 256.0f;
    res.z = static_cast<float>( rand() % 256 ) / 256.0f;
    return res;
}

void display()
{
    glClearColor( 0, 0, 0, 1 );
    glClear( GL_COLOR_BUFFER_BIT );
    
    s.push_back(Sphere(Point(0, 70, 150), 80, RGB_float(1, 1, 0)));
    s.push_back(Sphere(Point(-150, -20, 500), 150, RGB_float(1, 0.5, 0.8)));
    s.push_back(Sphere(Point(250, 0, 300), 100, RGB_float(0, 0.2, 0.8)));
    s.push_back(Sphere(Point(300, -200, 1000), 500, RGB_float(1, 0, 0)));
    
    float pixels[H][W][3];
    for(int j = 0; j < H; ++j ) {
        for(int i = 0; i < W; ++i ) {
            RGB_float color = rayCast(i, j);
            //RGB_float color = randomColor();
            pixels[j][i][0] = color.x;
            pixels[j][i][1] = color.y;
            pixels[j][i][2] = color.z;
        }
    }
    
    glDrawPixels( W, H, GL_RGB, GL_FLOAT, pixels);
    
    glutSwapBuffers();
}

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE );
    glutInitWindowSize( W, H );
    glutCreateWindow( "GLUT" );
    glutDisplayFunc( display );
    glutMainLoop();
    return 0;
}
