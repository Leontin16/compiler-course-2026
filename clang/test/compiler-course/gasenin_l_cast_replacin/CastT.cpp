// RUN: %clang_cc1 -std=c++17 \
// RUN:   -load %llvmshlibdir/gasenin_l_cast_replacin_ClangAST%pluginext \
// RUN:   -plugin cast_replace_plugin -fsyntax-only %s 2>&1 | FileCheck %s


void test_arithmetic() {
    double d = 3.14;
    int i = (int)d;

    double d2 = (double)i;

    long l = (long)i;

    float f = (float)d;
}

void test_void_pointer() {
    int x = 0;
    int *p = &x;

    void *vp = (void*)p;

    int *p2 = (int*)vp;
}

void test_const_cast() {
    int x = 0;
    const int *cp = &x;

    int *p = (int*)cp;

    const char *ccp = "hello";
    char *mp = (char*)ccp;

    const int ci = 42;
    int j = (int)ci;
}

void test_reinterpret() {
    int x = 42;
    int *ip = &x;

    char *cp = (char*)ip;

    long addr = (long)ip;

    int *ip2 = (int*)addr;

    char &cr = (char&)x;
}

struct Point    { int x, y; };
struct DataBlock{ float v[4]; };

class Base    { public: virtual ~Base() = default; };
class Derived : public Base { public: int id; };

class NpBase    { public: int val; };
class NpDerived : public NpBase { public: int extra; };

class VBase { public: int data; };
class VDerived : virtual public VBase {};

void test_hierarchy() {
    Derived derived_obj;
    Base *base_ptr = (Base *)&derived_obj;

    Base *b_ptr = new Derived();
    Derived *d_ptr = (Derived *)b_ptr;

    NpBase *np_base = new NpDerived();
    NpDerived *np_d = (NpDerived *)np_base;

    VDerived vd;
    VBase *vb = (VBase *)&vd;

    Point pt = {1, 2};
    DataBlock *data = (DataBlock *)&pt;

    const Point cpt = {0, 0};
    Point *mut_pt = (Point *)&cpt;
}

void test_to_bool() {
    int    i   = 1;
    double d   = 3.14;
    int   *p   = &i;

    bool bi = (bool)i;
    bool bd = (bool)d;
    bool bp = (bool)p;
}

void test_to_void() {
    int x = 0;
    (void)x;
}

struct Holder { int value; };

void test_null_casts() {
    int *np = (int*)0;

    int Holder::*mp = (int Holder::*)0;
}

struct MpBase    { int a; };
struct MpDerived { int b; };

void test_member_pointer_reinterpret() {
    int MpBase::*mbp = &MpBase::a;
    int MpDerived::*mdp = (int MpDerived::*)mbp;
}

#define MACRO_CAST(T, v) (T)(v)

void test_macro_skip() {
    double d = 2.71;
    int m = MACRO_CAST(int, d);
}
