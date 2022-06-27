#pragma once

#include <cmath>
#include <sstream>
#include <vector>

class Vec3
{
public:
    double x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z=0) : x(x), y(y), z(z) {}
    Vec3 operator-(const Vec3& v) {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }
    Vec3 operator+(const Vec3& v) {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }
    Vec3 operator/(const double t) {
        return Vec3(x / t, y / t, z / t);
    }
    Vec3 addX(float x){
        return Vec3((*this).x + x, (*this).y);
    }
    Vec3 addY(float y){
        return Vec3((*this).x, (*this).y + y);
    }
    void operator+=(const Vec3 &v){
        x += v.x;
        y += v.y;
        z += v.z;
    }
    void operator*=(const double t){
        x *= t;
        y *= t;
        z *= t;
    }
    void operator/=(const double t){
        return *this *= 1 / t;
    }
    bool operator!=(Vec3 &v){
        return x != v.x || y != v.y || z != v.z;
    }
    bool operator==(Vec3 &v){
        return x == v.x & y == v.y && z == v.z;
    }
    double length() const{
        return sqrt(lengthSquared());
    }
    double lengthSquared() const{
        return x * x + y * y + z * z;
    }
    std::string str() {
        std::stringstream ss;
        ss << x << ", " << y << ", " << ", ";
        return ss.str();
    }
    Vec3 midpoint(const Vec3& v){
        return (*this + v) / 2;
    }
    // get position 1/3 of the way between this vec and another
    Vec3 thirdpoint(const Vec3& v){
        return (*this + v) / 3;
    }
    Vec3 diff(Vec3 v){
        return v - *this;
    }
};

class Vec3Array{
    std::vector<Vec3> m_vecs;
    float* m_floats;
public:
    Vec3Array() {}
    Vec3Array(const std::vector<Vec3>& vecs) : m_vecs(vecs){}
    void operator=(const std::vector<Vec3>& vecs){
        m_vecs = vecs;
    }
    Vec3 operator[](int i){
        return m_vecs[i];
    }
    int size(){
        return m_vecs.size();
    }
    float* floats(){
        m_floats = new float[m_vecs.size()*2];
        int vecIt = 0;
        for(int i = 0; i < m_vecs.size()*2; i+=2){
            m_floats[i] = m_vecs[vecIt].x;
            m_floats[i+1] = m_vecs[vecIt].y;
            vecIt++;
        }
        return m_floats;
    }
    void free() {
        delete[] m_floats;
    }
};

using Point3 = Vec3;
using Colour = Vec3;