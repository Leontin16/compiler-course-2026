// RUN: %clang_cc1 -std=c++17 \
// RUN:   -load %llvmshlibdir/gasenin_l_cast_replacin_ClangAST%pluginext \
// RUN:   -plugin cast_replace_plugin -fsyntax-only %s 2>&1 | FileCheck %s

// ============================================================
// 1. Arithmetic casts  →  static_cast
// ============================================================
void test_arithmetic() {
    double d = 3.14;
    // CHECK: int i = static_cast<int>(d);
    int i = (int)d;

    // CHECK: double d2 = static_cast<double>(i);
    double d2 = (double)i;

    // CHECK: long l = static_cast<long>(i);
    long l = (long)i;

    // CK_FloatingCast: narrowing float conversion  →  static_cast
    // CHECK: float f = static_cast<float>(d);
    float f = (float)d;
}

// ============================================================
// 2. void* conversions  →  static_cast
// ============================================================
void test_void_pointer() {
    int x = 0;
    int *p = &x;

    // CHECK: void *vp = static_cast<void *>(p);
    void *vp = (void*)p;

    // CHECK: int *p2 = static_cast<int *>(vp);
    int *p2 = (int*)vp;
}

// ============================================================
// 3. const removal  →  const_cast
// ============================================================
void test_const_cast() {
    int x = 0;
    const int *cp = &x;

    // CK_NoOp (pointer): same pointee, different cv-qual  →  const_cast
    // CHECK: int *p = const_cast<int *>(cp);
    int *p = (int*)cp;

    const char *ccp = "hello";
    // CHECK: char *mp = const_cast<char *>(ccp);
    char *mp = (char*)ccp;

    // CK_NoOp (value): hasSameUnqualifiedType  →  const_cast
    const int ci = 42;
    // CHECK: int j = const_cast<int>(ci);
    int j = (int)ci;
}

// ============================================================
// 4. Unrelated pointer types  →  reinterpret_cast
// ============================================================
void test_reinterpret() {
    int x = 42;
    int *ip = &x;

    // CK_BitCast: unrelated pointer types  →  reinterpret_cast
    // CHECK: char *cp = reinterpret_cast<char *>(ip);
    char *cp = (char*)ip;

    // CK_PointerToIntegral  →  reinterpret_cast
    // CHECK: long addr = reinterpret_cast<long>(ip);
    long addr = (long)ip;

    // CK_IntegralToPointer  →  reinterpret_cast
    // CHECK: int *ip2 = reinterpret_cast<int *>(addr);
    int *ip2 = (int*)addr;

    // CK_LValueBitCast: cast reference to reference of unrelated type  →  reinterpret_cast
    // CHECK: char &cr = reinterpret_cast<char &>(x);
    char &cr = (char&)x;
}

// ============================================================
// 5. Class hierarchy
// ============================================================
struct Point    { int x, y; };
struct DataBlock{ float v[4]; };

class Base    { public: virtual ~Base() = default; };
class Derived : public Base { public: int id; };

// Non-polymorphic hierarchy for static downcast test
class NpBase    { public: int val; };
class NpDerived : public NpBase { public: int extra; };

// Virtual inheritance for CK_UncheckedDerivedToBase test
class VBase { public: int data; };
class VDerived : virtual public VBase {};

void test_hierarchy() {
    // Upcast  →  static_cast   (CK_DerivedToBase)
    Derived derived_obj;
    // CHECK: Base *base_ptr = static_cast<Base *>(&derived_obj);
    Base *base_ptr = (Base *)&derived_obj;

    // Downcast through polymorphic base  →  dynamic_cast  (CK_BaseToDerived, polymorphic)
    Base *b_ptr = new Derived();
    // CHECK: Derived *d_ptr = dynamic_cast<Derived *>(b_ptr);
    Derived *d_ptr = (Derived *)b_ptr;

    // Downcast through NON-polymorphic base  →  static_cast  (CK_BaseToDerived, non-polymorphic)
    NpBase *np_base = new NpDerived();
    // CHECK: NpDerived *np_d = static_cast<NpDerived *>(np_base);
    NpDerived *np_d = (NpDerived *)np_base;

    // Virtual base upcast  →  static_cast  (CK_UncheckedDerivedToBase)
    VDerived vd;
    // CHECK: VBase *vb = static_cast<VBase *>(&vd);
    VBase *vb = (VBase *)&vd;

    // Unrelated structs (BitCast)  →  reinterpret_cast
    Point pt = {1, 2};
    // CHECK: DataBlock *data = reinterpret_cast<DataBlock *>(&pt);
    DataBlock *data = (DataBlock *)&pt;

    // const removal on user type  →  const_cast
    const Point cpt = {0, 0};
    // CHECK: Point *mut_pt = const_cast<Point *>(&cpt);
    Point *mut_pt = (Point *)&cpt;
}

// ============================================================
// 6. To-boolean casts  →  static_cast
// ============================================================
void test_to_bool() {
    int    i   = 1;
    double d   = 3.14;
    int   *p   = &i;

    // CK_IntegralToBoolean  →  static_cast
    // CHECK: bool bi = static_cast<bool>(i);
    bool bi = (bool)i;

    // CK_FloatingToBoolean  →  static_cast
    // CHECK: bool bd = static_cast<bool>(d);
    bool bd = (bool)d;

    // CK_PointerToBoolean  →  static_cast
    // CHECK: bool bp = static_cast<bool>(p);
    bool bp = (bool)p;
}

// ============================================================
// 7. CK_ToVoid  →  static_cast
// ============================================================
void test_to_void() {
    int x = 0;
    // Discarded-value expression via cast  →  static_cast
    // CHECK: static_cast<void>(x);
    (void)x;
}

// ============================================================
// 8. Null pointer / null member-pointer  →  static_cast
// ============================================================
struct Holder { int value; };

void test_null_casts() {
    // CK_NullToPointer  →  static_cast
    // CHECK: int *np = static_cast<int *>(0);
    int *np = (int*)0;

    // CK_NullToMemberPointer  →  static_cast
    // CHECK: int Holder::*mp = static_cast<int Holder::*>(0);
    int Holder::*mp = (int Holder::*)0;
}

// ============================================================
// 9. CK_ReinterpretMemberPointer  →  reinterpret_cast
// ============================================================
struct MpBase    { int a; };
struct MpDerived { int b; };   // unrelated — forces reinterpret

void test_member_pointer_reinterpret() {
    int MpBase::*mbp = &MpBase::a;
    // Casting between member pointers of unrelated classes  →  reinterpret_cast
    // CHECK: int MpDerived::*mdp = reinterpret_cast<int MpDerived::*>(mbp);
    int MpDerived::*mdp = (int MpDerived::*)mbp;
}

// ============================================================
// 10. Macro expansion  →  plugin must NOT rewrite
// ============================================================
#define MACRO_CAST(T, v) (T)(v)

void test_macro_skip() {
    double d = 2.71;
    // The cast is inside a macro expansion — visitor must skip it.
    // CHECK: int m = MACRO_CAST(int, d);
    int m = MACRO_CAST(int, d);
}
