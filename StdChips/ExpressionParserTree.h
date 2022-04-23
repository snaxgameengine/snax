// SnaX Game Engine - https://github.com/snaxgameengine/snax
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "M3DCore/MString.h"
#include "M3DCore/Containers.h"
#include <memory>
#include <DirectXMath.h>
#include <functional>
#include <vector>


using namespace DirectX;

namespace m3d
{
namespace expr {


struct ExprNotImplException {};
struct InvalidDimensionException {};

struct ExprNode;
struct OperandNode;
struct ValueNode;
struct VectorNode;
struct Vector4Node;
struct Vector3Node;
struct Vector2Node;
struct MatrixNode;
struct Matrix44Node;
struct Matrix33Node;
struct Matrix43Node;

class ExpressionCallback
{
public:
	ExpressionCallback() {}
	virtual ~ExpressionCallback() {}
	virtual uint32 Register(const String &s) = 0;
	virtual OperandNode *GetOperand(uint32 index) = 0;
};

typedef std::unique_ptr<ExprNode> ExprNodePtr;
typedef List<ExprNodePtr> ExprNodePtrVector;

struct ExprNode
{
	ExprNode() {}
	ExprNode(const ExprNode &rhs) {}
	virtual ~ExprNode() {}

	virtual const OperandNode *operator()() const = 0;
	virtual bool isOperator() const = 0;
	virtual ExprNode *duplicate() const = 0;
	virtual bool build(ExprNodePtrVector &v) { v.pop_back(); return true; }
//	virtual String hlsl() const = 0;
};

typedef std::function<ExprNode*()> ExprNodeFactoryFunc;

struct OperandNode : ExprNode
{
	bool isOperator() const override { return false; }

	virtual const ValueNode *asValue() const { return nullptr; }
	virtual const VectorNode *asVector() const { return nullptr; }
	virtual const MatrixNode *asMatrix() const { return nullptr; }

	virtual const OperandNode *neg() const { throw ExprNotImplException(); }
	virtual const OperandNode *pos() const { throw ExprNotImplException(); }
	virtual const OperandNode *Not() const { throw ExprNotImplException(); }
	virtual const OperandNode *bwNot() const { throw ExprNotImplException(); }
	virtual const OperandNode *saturate() const { throw ExprNotImplException(); }
	virtual const OperandNode *sin() const { throw ExprNotImplException(); }
	virtual const OperandNode *cos() const { throw ExprNotImplException(); }
	virtual const OperandNode *tan() const { throw ExprNotImplException(); }
	virtual const OperandNode *asin() const { throw ExprNotImplException(); }
	virtual const OperandNode *acos() const { throw ExprNotImplException(); }
	virtual const OperandNode *atan() const { throw ExprNotImplException(); }
	virtual const OperandNode *abs() const { throw ExprNotImplException(); }
	virtual const OperandNode *sqrt() const { throw ExprNotImplException(); }
	virtual const OperandNode *rsqrt() const { throw ExprNotImplException(); }
	virtual const OperandNode *floor() const { throw ExprNotImplException(); }
	virtual const OperandNode *ceil() const { throw ExprNotImplException(); }
	virtual const OperandNode *round() const { throw ExprNotImplException(); }
	virtual const OperandNode *sinh() const { throw ExprNotImplException(); }
	virtual const OperandNode *cosh() const { throw ExprNotImplException(); }
	virtual const OperandNode *tanh() const { throw ExprNotImplException(); }
	virtual const OperandNode *log10() const { throw ExprNotImplException(); }
	virtual const OperandNode *log() const { throw ExprNotImplException(); }
	virtual const OperandNode *log2() const { throw ExprNotImplException(); }
	virtual const OperandNode *exp() const { throw ExprNotImplException(); }
	virtual const OperandNode *exp2() const { throw ExprNotImplException(); }
	virtual const OperandNode *frac() const { throw ExprNotImplException(); }
	virtual const OperandNode *sign() const { throw ExprNotImplException(); }
	virtual const OperandNode *rad() const { throw ExprNotImplException(); }
	virtual const OperandNode *deg() const { throw ExprNotImplException(); }
	virtual const OperandNode *isNaN() const { throw ExprNotImplException(); }
	virtual const OperandNode *isInf() const { throw ExprNotImplException(); }
	virtual const OperandNode *trunc() const { throw ExprNotImplException(); }
	virtual const OperandNode *all() const { throw ExprNotImplException(); }
	virtual const OperandNode *any() const { throw ExprNotImplException(); }
	virtual const OperandNode *length() const { throw ExprNotImplException(); }
	virtual const OperandNode *normalize() const { throw ExprNotImplException(); }
	virtual const OperandNode *determinant() const { throw ExprNotImplException(); }
	virtual const OperandNode *transpose() const { throw ExprNotImplException(); }
	virtual const OperandNode *inverse() const { throw ExprNotImplException(); }
	virtual const OperandNode *swizzle(uint32 mask) const { throw ExprNotImplException(); }

	virtual const OperandNode *add(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *sub(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mul(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *div(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *intMod(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *less(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *lessEq(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greater(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greaterEq(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *equal(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *notEqual(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *And(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *Or(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *min(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *max(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *pow(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *fmod(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *bwShiftLeft(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *bwShiftRight(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *bwAnd(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *bwOr(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *bwXor(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *atan2(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *ldexp(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *step(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *dot(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *cross(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *distance(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mulFunc(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *reflect(const ValueNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *subscript(const ValueNode *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *add(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *sub(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mul(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *div(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *intMod(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *less(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *lessEq(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greater(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greaterEq(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *equal(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *notEqual(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *And(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *Or(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *min(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *max(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *pow(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *fmod(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *atan2(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *ldexp(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *step(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *dot(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *distance(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mulFunc(const Vector4Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *reflect(const Vector4Node *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *add(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *sub(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mul(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *div(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *intMod(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *less(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *lessEq(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greater(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greaterEq(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *equal(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *notEqual(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *And(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *Or(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *min(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *max(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *pow(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *fmod(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *atan2(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *ldexp(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *step(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *dot(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *cross(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *distance(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mulFunc(const Vector3Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *reflect(const Vector3Node *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *add(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *sub(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mul(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *div(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *intMod(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *less(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *lessEq(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greater(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greaterEq(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *equal(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *notEqual(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *And(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *Or(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *min(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *max(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *pow(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *fmod(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *atan2(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *ldexp(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *step(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *dot(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *distance(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mulFunc(const Vector2Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *reflect(const Vector2Node *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *add(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *sub(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mul(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *div(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *intMod(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *less(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *lessEq(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greater(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greaterEq(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *equal(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *notEqual(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *And(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *Or(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *min(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *max(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *pow(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *fmod(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *atan2(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *ldexp(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *step(const Matrix44Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mulFunc(const Matrix44Node *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *add(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *sub(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mul(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *div(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *intMod(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *less(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *lessEq(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greater(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *greaterEq(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *equal(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *notEqual(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *And(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *Or(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *min(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *max(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *pow(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *fmod(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *atan2(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *ldexp(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *step(const Matrix33Node *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *mulFunc(const Matrix33Node *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *b_add(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_sub(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_mul(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_div(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_intMod(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_less(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_lessEq(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_greater(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_greaterEq(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_equal(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_notEqual(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_and(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_or(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_min(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_max(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_pow(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_fmod(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_bwShiftLeft(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_bwShiftRight(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_bwAnd(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_bwOr(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_bwXor(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_atan2(const OperandNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_ldexp(const OperandNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_step(const OperandNode *b) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_dot(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_cross(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_distance(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_mulFunc(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_reflect(const OperandNode *a) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_subscript(const OperandNode *a) const { throw ExprNotImplException(); }
	
	virtual const OperandNode *clamp(const ValueNode *b, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *clamp(const Vector4Node *b, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *clamp(const Vector3Node *b, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *clamp(const Vector2Node *b, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *clamp(const Matrix44Node *b, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_clamp(const OperandNode *a, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_clamp(const OperandNode *a, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_clamp(const OperandNode *a, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_clamp(const OperandNode *a, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_clamp(const OperandNode *a, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *c_clamp(const OperandNode *a, const OperandNode *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *lerp(const ValueNode *b, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *lerp(const Vector4Node *b, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *lerp(const Vector3Node *b, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *lerp(const Vector2Node *b, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *lerp(const Matrix44Node *b, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_lerp(const OperandNode *a, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_lerp(const OperandNode *a, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_lerp(const OperandNode *a, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_lerp(const OperandNode *a, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_lerp(const OperandNode *a, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *c_lerp(const OperandNode *a, const OperandNode *b) const { throw ExprNotImplException(); }

//	virtual const OperandNode *lit(const OperandNode *b, const OperandNode *c) const { throw ExprNotImplException(); }

	virtual const OperandNode *mad(const ValueNode *b, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *mad(const Vector4Node *b, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *mad(const Vector3Node *b, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *mad(const Vector2Node *b, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *mad(const Matrix44Node *b, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_mad(const OperandNode *a, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_mad(const OperandNode *a, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_mad(const OperandNode *a, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_mad(const OperandNode *a, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_mad(const OperandNode *a, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *c_mad(const OperandNode *a, const OperandNode *b) const { throw ExprNotImplException(); }

	virtual const OperandNode *smoothstep(const ValueNode *b, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *smoothstep(const Vector4Node *b, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *smoothstep(const Vector3Node *b, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *smoothstep(const Vector2Node *b, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *smoothstep(const Matrix44Node *b, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_smoothstep(const OperandNode *a, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_smoothstep(const OperandNode *a, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_smoothstep(const OperandNode *a, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_smoothstep(const OperandNode *a, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_smoothstep(const OperandNode *a, const Matrix44Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *c_smoothstep(const OperandNode *a, const OperandNode *b) const { throw ExprNotImplException(); }

//	virtual const OperandNode *refract(const OperandNode *b, const OperandNode *c) const { throw ExprNotImplException(); }

	virtual const OperandNode *ifThenElse(const ValueNode *b, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *ifThenElse(const Vector4Node *b, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *ifThenElse(const Vector3Node *b, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *ifThenElse(const Vector2Node *b, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_ifThenElse(const OperandNode *a, const ValueNode *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_ifThenElse(const OperandNode *a, const Vector4Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_ifThenElse(const OperandNode *a, const Vector3Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *b_ifThenElse(const OperandNode *a, const Vector2Node *c) const { throw ExprNotImplException(); }
	virtual const OperandNode *c_ifThenElse(const OperandNode *a, const OperandNode *b) const { throw ExprNotImplException(); }

};




struct ValueNode : OperandNode
{
	float64 v;

	ValueNode(float64 v = 0.0) : v(v) {}
	ValueNode(const ValueNode &rhs) : v(rhs.v) {}

	const OperandNode *operator()() const override { return this; }
	ExprNode *duplicate() const override { return new ValueNode(*this); }
	static ExprNode *create(float64 v) { return new ValueNode(v); }

	const ValueNode *asValue() const override { return this; }
	float64 value() const { return v; }

	const OperandNode *neg() const override;
	const OperandNode *pos() const override;
	const OperandNode *Not() const override;
	const OperandNode *bwNot() const override;
	const OperandNode *saturate() const override;
	const OperandNode *sin() const override;
	const OperandNode *cos() const override;
	const OperandNode *tan() const override;
	const OperandNode *asin() const override;
	const OperandNode *acos() const override;
	const OperandNode *atan() const override;
	const OperandNode *abs() const override;
	const OperandNode *sqrt() const override;
	const OperandNode *rsqrt() const override;
	const OperandNode *floor() const override;
	const OperandNode *ceil() const override;
	const OperandNode *round() const override;
	const OperandNode *sinh() const override;
	const OperandNode *cosh() const override;
	const OperandNode *tanh() const override;
	const OperandNode *log() const override;
	const OperandNode *log10() const override;
	const OperandNode *log2() const override;
	const OperandNode *exp() const override;
	const OperandNode *exp2() const override;
	const OperandNode *frac() const override;
	const OperandNode *sign() const override;
	const OperandNode *rad() const override;
	const OperandNode *deg() const override;
	const OperandNode *isNaN() const override;
	const OperandNode *isInf() const override;
	const OperandNode *trunc() const override;

	const OperandNode *add(const ValueNode *b) const override;
	const OperandNode *sub(const ValueNode *b) const override;
	const OperandNode *mul(const ValueNode *b) const override;
	const OperandNode *div(const ValueNode *b) const override;
	const OperandNode *intMod(const ValueNode *b) const override;
	const OperandNode *less(const ValueNode *b) const override;
	const OperandNode *lessEq(const ValueNode *b) const override;
	const OperandNode *greater(const ValueNode *b) const override;
	const OperandNode *greaterEq(const ValueNode *b) const override;
	const OperandNode *equal(const ValueNode *b) const override;
	const OperandNode *notEqual(const ValueNode *b) const override;
	const OperandNode *And(const ValueNode *b) const override;
	const OperandNode *Or(const ValueNode *b) const override;
	const OperandNode *min(const ValueNode *b) const override;
	const OperandNode *max(const ValueNode *b) const override;
	const OperandNode *pow(const ValueNode *b) const override;
	const OperandNode *fmod(const ValueNode *b) const override;
	const OperandNode *bwShiftLeft(const ValueNode *b) const override;
	const OperandNode *bwShiftRight(const ValueNode *b) const override;
	const OperandNode *bwAnd(const ValueNode *b) const override;
	const OperandNode *bwOr(const ValueNode *b) const override;
	const OperandNode *bwXor(const ValueNode *b) const override;
	const OperandNode *atan2(const ValueNode *b) const override;
	const OperandNode *ldexp(const ValueNode *b) const override;
	const OperandNode *step(const ValueNode *b) const override;
	const OperandNode *mulFunc(const ValueNode *b) const override;

	const OperandNode *add(const Vector4Node *b) const override;
	const OperandNode *sub(const Vector4Node *b) const override;
	const OperandNode *mul(const Vector4Node *b) const override;
	const OperandNode *div(const Vector4Node *b) const override;
	const OperandNode *intMod(const Vector4Node *b) const override;
	const OperandNode *less(const Vector4Node *b) const override;
	const OperandNode *lessEq(const Vector4Node *b) const override;
	const OperandNode *greater(const Vector4Node *b) const override;
	const OperandNode *greaterEq(const Vector4Node *b) const override;
	const OperandNode *equal(const Vector4Node *b) const override;
	const OperandNode *notEqual(const Vector4Node *b) const override;
	const OperandNode *And(const Vector4Node *b) const override;
	const OperandNode *Or(const Vector4Node *b) const override;
	const OperandNode *min(const Vector4Node *b) const override;
	const OperandNode *max(const Vector4Node *b) const override;
	const OperandNode *pow(const Vector4Node *b) const override;
	const OperandNode *fmod(const Vector4Node *b) const override;
	const OperandNode *atan2(const Vector4Node *b) const override;
	const OperandNode *ldexp(const Vector4Node *b) const override;
	const OperandNode *step(const Vector4Node *b) const override;
	const OperandNode *dot(const Vector4Node *b) const override;
	const OperandNode *distance(const Vector4Node *b) const override;
	const OperandNode *mulFunc(const Vector4Node *b) const override;
	const OperandNode *reflect(const Vector4Node *b) const override;

	const OperandNode *add(const Vector3Node *b) const override;
	const OperandNode *sub(const Vector3Node *b) const override;
	const OperandNode *mul(const Vector3Node *b) const override;
	const OperandNode *div(const Vector3Node *b) const override;
	const OperandNode *intMod(const Vector3Node *b) const override;
	const OperandNode *less(const Vector3Node *b) const override;
	const OperandNode *lessEq(const Vector3Node *b) const override;
	const OperandNode *greater(const Vector3Node *b) const override;
	const OperandNode *greaterEq(const Vector3Node *b) const override;
	const OperandNode *equal(const Vector3Node *b) const override;
	const OperandNode *notEqual(const Vector3Node *b) const override;
	const OperandNode *And(const Vector3Node *b) const override;
	const OperandNode *Or(const Vector3Node *b) const override;
	const OperandNode *min(const Vector3Node *b) const override;
	const OperandNode *max(const Vector3Node *b) const override;
	const OperandNode *pow(const Vector3Node *b) const override;
	const OperandNode *fmod(const Vector3Node *b) const override;
	const OperandNode *atan2(const Vector3Node *b) const override;
	const OperandNode *ldexp(const Vector3Node *b) const override;
	const OperandNode *step(const Vector3Node *b) const override;
	const OperandNode *dot(const Vector3Node *b) const override;
	const OperandNode *cross(const Vector3Node *b) const override;
	const OperandNode *distance(const Vector3Node *b) const override;
	const OperandNode *mulFunc(const Vector3Node *b) const override;
	const OperandNode *reflect(const Vector3Node *b) const override;

	const OperandNode *add(const Vector2Node *b) const override;
	const OperandNode *sub(const Vector2Node *b) const override;
	const OperandNode *mul(const Vector2Node *b) const override;
	const OperandNode *div(const Vector2Node *b) const override;
	const OperandNode *intMod(const Vector2Node *b) const override;
	const OperandNode *less(const Vector2Node *b) const override;
	const OperandNode *lessEq(const Vector2Node *b) const override;
	const OperandNode *greater(const Vector2Node *b) const override;
	const OperandNode *greaterEq(const Vector2Node *b) const override;
	const OperandNode *equal(const Vector2Node *b) const override;
	const OperandNode *notEqual(const Vector2Node *b) const override;
	const OperandNode *And(const Vector2Node *b) const override;
	const OperandNode *Or(const Vector2Node *b) const override;
	const OperandNode *min(const Vector2Node *b) const override;
	const OperandNode *max(const Vector2Node *b) const override;
	const OperandNode *pow(const Vector2Node *b) const override;
	const OperandNode *fmod(const Vector2Node *b) const override;
	const OperandNode *atan2(const Vector2Node *b) const override;
	const OperandNode *ldexp(const Vector2Node *b) const override;
	const OperandNode *step(const Vector2Node *b) const override;
	const OperandNode *dot(const Vector2Node *b) const override;
	const OperandNode *distance(const Vector2Node *b) const override;
	const OperandNode *mulFunc(const Vector2Node *b) const override;
	const OperandNode *reflect(const Vector2Node *b) const override;

	const OperandNode *add(const Matrix44Node *b) const override;
	const OperandNode *sub(const Matrix44Node *b) const override;
	const OperandNode *mul(const Matrix44Node *b) const override;
	const OperandNode *div(const Matrix44Node *b) const override;
	const OperandNode *intMod(const Matrix44Node *b) const override;
	const OperandNode *less(const Matrix44Node *b) const override;
	const OperandNode *lessEq(const Matrix44Node *b) const override;
	const OperandNode *greater(const Matrix44Node *b) const override;
	const OperandNode *greaterEq(const Matrix44Node *b) const override;
	const OperandNode *equal(const Matrix44Node *b) const override;
	const OperandNode *notEqual(const Matrix44Node *b) const override;
	const OperandNode *And(const Matrix44Node *b) const override;
	const OperandNode *Or(const Matrix44Node *b) const override;
	const OperandNode *min(const Matrix44Node *b) const override;
	const OperandNode *max(const Matrix44Node *b) const override;
	const OperandNode *pow(const Matrix44Node *b) const override;
	const OperandNode *fmod(const Matrix44Node *b) const override;
	const OperandNode *atan2(const Matrix44Node *b) const override;
	const OperandNode *ldexp(const Matrix44Node *b) const override;
	const OperandNode *step(const Matrix44Node *b) const override;
	const OperandNode *mulFunc(const Matrix44Node *b) const override;

	const OperandNode *b_add(const OperandNode *a) const override { return a->add(this); }
	const OperandNode *b_sub(const OperandNode *a) const override { return a->sub(this); }
	const OperandNode *b_mul(const OperandNode *a) const override { return a->mul(this); }
	const OperandNode *b_div(const OperandNode *a) const override { return a->div(this); }
	const OperandNode *b_intMod(const OperandNode *a) const override { return a->intMod(this); }
	const OperandNode *b_less(const OperandNode *a) const override { return a->less(this); }
	const OperandNode *b_lessEq(const OperandNode *a) const override { return a->lessEq(this); }
	const OperandNode *b_greater(const OperandNode *a) const override { return a->greater(this); }
	const OperandNode *b_greaterEq(const OperandNode *a) const override { return a->greaterEq(this); }
	const OperandNode *b_equal(const OperandNode *a) const override { return a->equal(this); }
	const OperandNode *b_notEqual(const OperandNode *a) const override { return a->notEqual(this); }
	const OperandNode *b_and(const OperandNode *a) const override { return a->And(this); }
	const OperandNode *b_or(const OperandNode *a) const override { return a->Or(this); }
	const OperandNode *b_min(const OperandNode *a) const override { return a->min(this); }
	const OperandNode *b_max(const OperandNode *a) const override { return a->max(this); }
	const OperandNode *b_pow(const OperandNode *a) const override { return a->pow(this); }
	const OperandNode *b_fmod(const OperandNode *a) const override { return a->fmod(this); }
	const OperandNode *b_bwShiftLeft(const OperandNode *a) const override { return a->bwShiftLeft(this); }
	const OperandNode *b_bwShiftRight(const OperandNode *a) const override { return a->bwShiftRight(this); }
	const OperandNode *b_bwAnd(const OperandNode *a) const override { return a->bwAnd(this); }
	const OperandNode *b_bwOr(const OperandNode *a) const override { return a->bwOr(this); }
	const OperandNode *b_bwXor(const OperandNode *a) const override { return a->bwXor(this); }
	const OperandNode *b_atan2(const OperandNode *a) const override { return a->atan2(this); }
	const OperandNode *b_ldexp(const OperandNode *a) const override { return a->ldexp(this); }
	const OperandNode *b_step(const OperandNode *a) const override { return a->step(this); }
	const OperandNode *b_dot(const OperandNode *a) const override { return a->dot(this); }
	const OperandNode *b_cross(const OperandNode *a) const override { return a->dot(this); }
	const OperandNode *b_distance(const OperandNode *a) const override { return a->distance(this); }
	const OperandNode *b_mulFunc(const OperandNode *a) const override { return a->mulFunc(this); }
	const OperandNode *b_reflect(const OperandNode *a) const override { return a->reflect(this); }
	const OperandNode *b_subscript(const OperandNode *a) const override { return a->subscript(this); }

	const OperandNode *clamp(const ValueNode *b, const ValueNode *c) const override;
	const OperandNode *clamp(const Vector4Node *b, const Vector4Node *c) const override;
	const OperandNode *clamp(const Vector3Node *b, const Vector3Node *c) const override;
	const OperandNode *clamp(const Vector2Node *b, const Vector2Node *c) const override;
	const OperandNode *b_clamp(const OperandNode *a, const ValueNode *c) const override;
	const OperandNode *b_clamp(const OperandNode *a, const Vector4Node *c) const override;
	const OperandNode *b_clamp(const OperandNode *a, const Vector3Node *c) const override;
	const OperandNode *b_clamp(const OperandNode *a, const Vector2Node *c) const override;
	const OperandNode *c_clamp(const OperandNode *a, const OperandNode *b) const override;

	const OperandNode *lerp(const ValueNode *b, const ValueNode *c) const override;
	const OperandNode *lerp(const Vector4Node *b, const Vector4Node *c) const override;
	const OperandNode *lerp(const Vector3Node *b, const Vector3Node *c) const override;
	const OperandNode *lerp(const Vector2Node *b, const Vector2Node *c) const override;
	const OperandNode *b_lerp(const OperandNode *a, const ValueNode *c) const override;
	const OperandNode *b_lerp(const OperandNode *a, const Vector4Node *c) const override;
	const OperandNode *b_lerp(const OperandNode *a, const Vector3Node *c) const override;
	const OperandNode *b_lerp(const OperandNode *a, const Vector2Node *c) const override;
	const OperandNode *c_lerp(const OperandNode *a, const OperandNode *b) const override;

	const OperandNode *mad(const ValueNode *b, const ValueNode *c) const override;
	const OperandNode *mad(const Vector4Node *b, const Vector4Node *c) const override;
	const OperandNode *mad(const Vector3Node *b, const Vector3Node *c) const override;
	const OperandNode *mad(const Vector2Node *b, const Vector2Node *c) const override;
	const OperandNode *b_mad(const OperandNode *a, const ValueNode *c) const override;
	const OperandNode *b_mad(const OperandNode *a, const Vector4Node *c) const override;
	const OperandNode *b_mad(const OperandNode *a, const Vector3Node *c) const override;
	const OperandNode *b_mad(const OperandNode *a, const Vector2Node *c) const override;
	const OperandNode *c_mad(const OperandNode *a, const OperandNode *b) const override;

	const OperandNode *smoothstep(const ValueNode *b, const ValueNode *c) const override;
	const OperandNode *smoothstep(const Vector4Node *b, const Vector4Node *c) const override;
	const OperandNode *smoothstep(const Vector3Node *b, const Vector3Node *c) const override;
	const OperandNode *smoothstep(const Vector2Node *b, const Vector2Node *c) const override;
	const OperandNode *b_smoothstep(const OperandNode *a, const ValueNode *c) const override;
	const OperandNode *b_smoothstep(const OperandNode *a, const Vector4Node *c) const override;
	const OperandNode *b_smoothstep(const OperandNode *a, const Vector3Node *c) const override;
	const OperandNode *b_smoothstep(const OperandNode *a, const Vector2Node *c) const override;
	const OperandNode *c_smoothstep(const OperandNode *a, const OperandNode *b) const override;

	const OperandNode *ifThenElse(const ValueNode *b, const ValueNode *c) const override;
	const OperandNode *ifThenElse(const Vector4Node *b, const Vector4Node *c) const override;
	const OperandNode *ifThenElse(const Vector3Node *b, const Vector3Node *c) const override;
	const OperandNode *ifThenElse(const Vector2Node *b, const Vector2Node *c) const override;
	const OperandNode *b_ifThenElse(const OperandNode *a, const ValueNode *c) const override;
	const OperandNode *b_ifThenElse(const OperandNode *a, const Vector4Node *c) const override;
	const OperandNode *b_ifThenElse(const OperandNode *a, const Vector3Node *c) const override;
	const OperandNode *b_ifThenElse(const OperandNode *a, const Vector2Node *c) const override;
	const OperandNode *c_ifThenElse(const OperandNode *a, const OperandNode *b) const override;
};



struct VectorNode : OperandNode
{
	XMVECTOR v;

	VectorNode(const VectorNode &rhs) : v(rhs.v) {}
	VectorNode(FXMVECTOR v) : v(v) {}

	const OperandNode *operator()() const override { return this; }

	const VectorNode *asVector() const override { return this; }

	virtual VectorNode *duplicate(FXMVECTOR v) const = 0;
	virtual XMFLOAT4 vector() const = 0;
	virtual uint32 ncomp() const = 0;

	const OperandNode *neg() const override;
	const OperandNode *pos() const override;
	const OperandNode *Not() const override;
	const OperandNode *saturate() const override;
	const OperandNode *sin() const override;
	const OperandNode *cos() const override;
	const OperandNode *tan() const override;
	const OperandNode *asin() const override;
	const OperandNode *acos() const override;
	const OperandNode *atan() const override;
	const OperandNode *abs() const override;
	const OperandNode *sqrt() const override;
	const OperandNode *rsqrt() const override;
	const OperandNode *floor() const override;
	const OperandNode *ceil() const override;
	const OperandNode *round() const override;
	const OperandNode *sinh() const override;
	const OperandNode *cosh() const override;
	const OperandNode *tanh() const override;
	const OperandNode *log10() const override;
	const OperandNode *log() const override;
	const OperandNode *log2() const override;
	const OperandNode *exp() const override;
	const OperandNode *exp2() const override;
	const OperandNode *frac() const override;
	const OperandNode *sign() const override;
	const OperandNode *rad() const override;
	const OperandNode *deg() const override;
	const OperandNode *isNaN() const override;
	const OperandNode *isInf() const override;
	const OperandNode *trunc() const override;
	const OperandNode *swizzle(uint32 mask) const override;

	const OperandNode *subscript(const ValueNode *b) const override;

	const OperandNode *add(const VectorNode *b) const;
	const OperandNode *sub(const VectorNode *b) const;
	const OperandNode *mul(const VectorNode *b) const;
	const OperandNode *div(const VectorNode *b) const;
	const OperandNode *intMod(const VectorNode *b) const;
	const OperandNode *less(const VectorNode *b) const;
	const OperandNode *lessEq(const VectorNode *b) const;
	const OperandNode *greater(const VectorNode *b) const;
	const OperandNode *greaterEq(const VectorNode *b) const;
	const OperandNode *equal(const VectorNode *b) const;
	const OperandNode *notEqual(const VectorNode *b) const;
	const OperandNode *And(const VectorNode *b) const;
	const OperandNode *Or(const VectorNode *b) const;
	const OperandNode *min(const VectorNode *b) const;
	const OperandNode *max(const VectorNode *b) const;
	const OperandNode *pow(const VectorNode *b) const;
	const OperandNode *fmod(const VectorNode *b) const;
	const OperandNode *atan2(const VectorNode *b) const;
	const OperandNode *ldexp(const VectorNode *b) const;
	const OperandNode *step(const VectorNode *b) const;
};

struct Vector4Node : VectorNode
{
	Vector4Node(const XMFLOAT4 &v) : VectorNode(XMLoadFloat4(&v)) {}
	Vector4Node(FXMVECTOR v) : VectorNode(v) {}
	Vector4Node(const ValueNode *rhs) : VectorNode(XMVectorReplicate((float32)rhs->v)) { }

	ExprNode *duplicate() const override { return new Vector4Node(*this); }
	static ExprNode *create(const XMFLOAT4 &v) { return new Vector4Node(v); }

	VectorNode *duplicate(FXMVECTOR v) const override { return new Vector4Node(v); }
	XMFLOAT4 vector() const override { XMFLOAT4 f; XMStoreFloat4(&f, v); return f; }
	uint32 ncomp() const override { return 4; }


	const OperandNode *all() const override;
	const OperandNode *any() const override;
	const OperandNode *length() const override;
	const OperandNode *normalize() const override;

	const OperandNode *add(const ValueNode *b) const override;
	const OperandNode *sub(const ValueNode *b) const override;
	const OperandNode *mul(const ValueNode *b) const override;
	const OperandNode *div(const ValueNode *b) const override;
	const OperandNode *intMod(const ValueNode *b) const override;
	const OperandNode *less(const ValueNode *b) const override;
	const OperandNode *lessEq(const ValueNode *b) const override;
	const OperandNode *greater(const ValueNode *b) const override;
	const OperandNode *greaterEq(const ValueNode *b) const override;
	const OperandNode *equal(const ValueNode *b) const override;
	const OperandNode *notEqual(const ValueNode *b) const override;
	const OperandNode *And(const ValueNode *b) const override;
	const OperandNode *Or(const ValueNode *b) const override;
	const OperandNode *min(const ValueNode *b) const override;
	const OperandNode *max(const ValueNode *b) const override;
	const OperandNode *pow(const ValueNode *b) const override;
	const OperandNode *fmod(const ValueNode *b) const override;
	const OperandNode *atan2(const ValueNode *b) const override;
	const OperandNode *ldexp(const ValueNode *b) const override;
	const OperandNode *step(const ValueNode *b) const override;
	const OperandNode *dot(const ValueNode *b) const override;
	const OperandNode *distance(const ValueNode *b) const override;
	const OperandNode *mulFunc(const ValueNode *b) const override;
	const OperandNode *reflect(const ValueNode *b) const override;

	const OperandNode *add(const Vector4Node *b) const override;
	const OperandNode *sub(const Vector4Node *b) const override;
	const OperandNode *mul(const Vector4Node *b) const override;
	const OperandNode *div(const Vector4Node *b) const override;
	const OperandNode *intMod(const Vector4Node *b) const override;
	const OperandNode *less(const Vector4Node *b) const override;
	const OperandNode *lessEq(const Vector4Node *b) const override;
	const OperandNode *greater(const Vector4Node *b) const override;
	const OperandNode *greaterEq(const Vector4Node *b) const override;
	const OperandNode *equal(const Vector4Node *b) const override;
	const OperandNode *notEqual(const Vector4Node *b) const override;
	const OperandNode *And(const Vector4Node *b) const override;
	const OperandNode *Or(const Vector4Node *b) const override;
	const OperandNode *min(const Vector4Node *b) const override;
	const OperandNode *max(const Vector4Node *b) const override;
	const OperandNode *pow(const Vector4Node *b) const override;
	const OperandNode *fmod(const Vector4Node *b) const override;
	const OperandNode *atan2(const Vector4Node *b) const override;
	const OperandNode *ldexp(const Vector4Node *b) const override;
	const OperandNode *step(const Vector4Node *b) const override;
	const OperandNode *dot(const Vector4Node *b) const override;
	const OperandNode *distance(const Vector4Node *b) const override;
	const OperandNode *mulFunc(const Vector4Node *b) const override;
	const OperandNode *reflect(const Vector4Node *b) const override;

	const OperandNode *mul(const Matrix44Node *b) const override;
	const OperandNode *mulFunc(const Matrix44Node *b) const override;

	const OperandNode *b_add(const OperandNode *a) const override { return a->add(this); }
	const OperandNode *b_sub(const OperandNode *a) const override { return a->sub(this); }
	const OperandNode *b_mul(const OperandNode *a) const override { return a->mul(this); }
	const OperandNode *b_div(const OperandNode *a) const override { return a->div(this); }
	const OperandNode *b_intMod(const OperandNode *a) const override { return a->intMod(this); }
	const OperandNode *b_less(const OperandNode *a) const override { return a->less(this); }
	const OperandNode *b_lessEq(const OperandNode *a) const override { return a->lessEq(this); }
	const OperandNode *b_greater(const OperandNode *a) const override { return a->greater(this); }
	const OperandNode *b_greaterEq(const OperandNode *a) const override { return a->greaterEq(this); }
	const OperandNode *b_equal(const OperandNode *a) const override { return a->equal(this); }
	const OperandNode *b_notEqual(const OperandNode *a) const override { return a->notEqual(this); }
	const OperandNode *b_and(const OperandNode *a) const override { return a->And(this); }
	const OperandNode *b_or(const OperandNode *a) const override { return a->Or(this); }
	const OperandNode *b_min(const OperandNode *a) const override { return a->min(this); }
	const OperandNode *b_max(const OperandNode *a) const override { return a->max(this); }
	const OperandNode *b_pow(const OperandNode *a) const override { return a->pow(this); }
	const OperandNode *b_fmod(const OperandNode *a) const override { return a->fmod(this); }
	const OperandNode *b_atan2(const OperandNode *a) const override { return a->atan2(this); }
	const OperandNode *b_ldexp(const OperandNode *a) const override { return a->ldexp(this); }
	const OperandNode *b_step(const OperandNode *a) const override { return a->step(this); }
	const OperandNode *b_dot(const OperandNode *a) const override { return a->dot(this); }
	const OperandNode *b_distance(const OperandNode *a) const override { return a->distance(this); }
	const OperandNode *b_mulFunc(const OperandNode *a) const override { return a->mulFunc(this); }
	const OperandNode *b_reflect(const OperandNode *a) const override { return a->reflect(this); }

	const OperandNode *clamp(const Vector4Node *b, const Vector4Node *c) const override { return new Vector4Node(XMVectorClamp(v, b->v, c->v)); }
	const OperandNode *b_clamp(const OperandNode *a, const Vector4Node *c) const override { return a->clamp(this, c); }
	const OperandNode *c_clamp(const OperandNode *a, const OperandNode *b) const override { return b->b_clamp(a, this); }

	const OperandNode *lerp(const Vector4Node *b, const Vector4Node *c) const override { return new Vector4Node(XMVectorLerpV(v, b->v, c->v)); }
	const OperandNode *b_lerp(const OperandNode *a, const Vector4Node *c) const override { return a->lerp(this, c); }
	const OperandNode *c_lerp(const OperandNode *a, const OperandNode *b) const override { return b->b_lerp(a, this); }

	const OperandNode *mad(const Vector4Node *b, const Vector4Node *c) const override { return new Vector4Node(XMVectorMultiplyAdd(v, b->v, c->v)); }
	const OperandNode *b_mad(const OperandNode *a, const Vector4Node *c) const override { return a->mad(this, c); }
	const OperandNode *c_mad(const OperandNode *a, const OperandNode *b) const override { return b->b_mad(a, this); }

	const OperandNode *smoothstep(const Vector4Node *b, const Vector4Node *c) const override { XMVECTOR t = XMVectorSaturate(XMVectorDivide(XMVectorSubtract(c->v, v), XMVectorSubtract(b->v, v))); return new Vector4Node(XMVectorMultiply(XMVectorMultiply(t, t), XMVectorNegativeMultiplySubtract(t, XMVectorReplicate(2.0f), XMVectorReplicate(3.0f)))); }
	const OperandNode *b_smoothstep(const OperandNode *a, const Vector4Node *c) const override { return a->smoothstep(this, c); }
	const OperandNode *c_smoothstep(const OperandNode *a, const OperandNode *b) const override { return b->b_smoothstep(a, this); }

	const OperandNode *ifThenElse(const Vector4Node *b, const Vector4Node *c) const override { return new Vector4Node(XMVectorSelect(c->v, b->v, XMVectorNotEqual(v, XMVectorZero()))); }
	const OperandNode *b_ifThenElse(const OperandNode *a, const Vector4Node *c) const override { return a->ifThenElse(this, c); }
	const OperandNode *c_ifThenElse(const OperandNode *a, const OperandNode *b) const override { return b->b_ifThenElse(a, this); }
};

struct Vector3Node : VectorNode
{
	Vector3Node(const XMFLOAT3 &v) : VectorNode(XMLoadFloat3(&v)) {}
	Vector3Node(FXMVECTOR v) : VectorNode(v) {}
	Vector3Node(const ValueNode *rhs) : VectorNode(XMVectorReplicate((float32)rhs->v)) { }

	ExprNode *duplicate() const override { return new Vector3Node(*this); }
	static ExprNode *create(const XMFLOAT3 &v) { return new Vector3Node(v); }

	VectorNode *duplicate(FXMVECTOR v) const override { return new Vector3Node(v); }
	XMFLOAT4 vector() const override { XMFLOAT4 f; XMStoreFloat4(&f, v); f.w = 0.0f; return f; }
	uint32 ncomp() const override { return 3; }

	const OperandNode *all() const override { return new ValueNode(XMComparisonAllFalse(XMVector3EqualR(v, XMVectorZero())) ? 1.0f : 0.0f); }
	const OperandNode *any() const override { return new ValueNode(XMComparisonAnyFalse(XMVector3EqualR(v, XMVectorZero())) ? 1.0f : 0.0f); }
	const OperandNode *length() const override { return new ValueNode(XMVectorGetX(XMVector3Length(v))); }
	const OperandNode *normalize() const override { return duplicate(XMVector3Normalize(v)); }

	const OperandNode* add(const ValueNode* b) const override { Vector3Node t(b); return add(&t); }
	const OperandNode *sub(const ValueNode *b) const override { Vector3Node t(b); return sub(&t); }
	const OperandNode *mul(const ValueNode *b) const override { Vector3Node t(b); return mul(&t); }
	const OperandNode *div(const ValueNode *b) const override { Vector3Node t(b); return div(&t); }
	const OperandNode *intMod(const ValueNode *b) const override { Vector3Node t(b); return intMod(&t); }
	const OperandNode *less(const ValueNode *b) const override { Vector3Node t(b); return less(&t); }
	const OperandNode *lessEq(const ValueNode *b) const override { Vector3Node t(b); return lessEq(&t); }
	const OperandNode *greater(const ValueNode *b) const override { Vector3Node t(b); return greater(&t); }
	const OperandNode *greaterEq(const ValueNode *b) const override { Vector3Node t(b); return greaterEq(&t); }
	const OperandNode *equal(const ValueNode *b) const override { Vector3Node t(b); return equal(&t); }
	const OperandNode *notEqual(const ValueNode *b) const override { Vector3Node t(b); return notEqual(&t); }
	const OperandNode *And(const ValueNode *b) const override { Vector3Node t(b); return And(&t); }
	const OperandNode *Or(const ValueNode *b) const override { Vector3Node t(b); return Or(&t); }
	const OperandNode *min(const ValueNode *b) const override { Vector3Node t(b); return min(&t); }
	const OperandNode *max(const ValueNode *b) const override { Vector3Node t(b); return max(&t); }
	const OperandNode *pow(const ValueNode *b) const override { Vector3Node t(b); return pow(&t); }
	const OperandNode *fmod(const ValueNode *b) const override { Vector3Node t(b); return fmod(&t); }
	const OperandNode *atan2(const ValueNode *b) const override { Vector3Node t(b); return atan2(&t); }
	const OperandNode *ldexp(const ValueNode *b) const override { Vector3Node t(b); return ldexp(&t); }
	const OperandNode *step(const ValueNode *b) const override { Vector3Node t(b); return step(&t); }
	const OperandNode *dot(const ValueNode *b) const override { Vector3Node t(b); return dot(&t); }
	const OperandNode *cross(const ValueNode *b) const override { Vector3Node t(b); return cross(&t); }
	const OperandNode *distance(const ValueNode *b) const override { Vector3Node t(b); return distance(&t); }
	const OperandNode *mulFunc(const ValueNode *b) const override { Vector3Node t(b); return mulFunc(&t); }
	const OperandNode *reflect(const ValueNode *b) const override { Vector3Node t(b); return reflect(&t); }

	const OperandNode *add(const Vector3Node *b) const override { return VectorNode::add(b); }
	const OperandNode *sub(const Vector3Node *b) const override { return VectorNode::sub(b); }
	const OperandNode *mul(const Vector3Node *b) const override { return VectorNode::mul(b); }
	const OperandNode *div(const Vector3Node *b) const override { return VectorNode::div(b); }
	const OperandNode *intMod(const Vector3Node *b) const override { return VectorNode::intMod(b); }
	const OperandNode *less(const Vector3Node *b) const override { return VectorNode::less(b); }
	const OperandNode *lessEq(const Vector3Node *b) const override { return VectorNode::lessEq(b); }
	const OperandNode *greater(const Vector3Node *b) const override { return VectorNode::greater(b); }
	const OperandNode *greaterEq(const Vector3Node *b) const override { return VectorNode::greaterEq(b); }
	const OperandNode *equal(const Vector3Node *b) const override { return VectorNode::equal(b); }
	const OperandNode *notEqual(const Vector3Node *b) const override { return VectorNode::notEqual(b); }
	const OperandNode *And(const Vector3Node *b) const override { return VectorNode::And(b); }
	const OperandNode *Or(const Vector3Node *b) const override { return VectorNode::Or(b); }
	const OperandNode *min(const Vector3Node *b) const override { return VectorNode::min(b); }
	const OperandNode *max(const Vector3Node *b) const override { return VectorNode::max(b); }
	const OperandNode *pow(const Vector3Node *b) const override { return VectorNode::pow(b); }
	const OperandNode *fmod(const Vector3Node *b) const override { return VectorNode::fmod(b); }
	const OperandNode *atan2(const Vector3Node *b) const override { return VectorNode::atan2(b); }
	const OperandNode *ldexp(const Vector3Node *b) const override { return VectorNode::ldexp(b); }
	const OperandNode *step(const Vector3Node *b) const override { return VectorNode::step(b); }
	const OperandNode *dot(const Vector3Node *b) const override { return new ValueNode(XMVectorGetX(XMVector3Dot(v, b->v))); }
	const OperandNode *cross(const Vector3Node *b) const override { return duplicate(XMVector3Cross(v, b->v)); }
	const OperandNode *distance(const Vector3Node *b) const override { return new ValueNode(XMVectorGetX(XMVector3Length(v - b->v))); }
	const OperandNode *mulFunc(const Vector3Node *b) const override { return mul(b); }
	const OperandNode *reflect(const Vector3Node *b) const override { return duplicate(XMVector3Reflect(v, b->v)); }

	const OperandNode *mul(const Matrix33Node *b) const override;
	const OperandNode *mulFunc(const Matrix33Node *b) const override;

	const OperandNode *b_add(const OperandNode *a) const override { return a->add(this); }
	const OperandNode *b_sub(const OperandNode *a) const override { return a->sub(this); }
	const OperandNode *b_mul(const OperandNode *a) const override { return a->mul(this); }
	const OperandNode *b_div(const OperandNode *a) const override { return a->div(this); }
	const OperandNode *b_intMod(const OperandNode *a) const override { return a->intMod(this); }
	const OperandNode *b_less(const OperandNode *a) const override { return a->less(this); }
	const OperandNode *b_lessEq(const OperandNode *a) const override { return a->lessEq(this); }
	const OperandNode *b_greater(const OperandNode *a) const override { return a->greater(this); }
	const OperandNode *b_greaterEq(const OperandNode *a) const override { return a->greaterEq(this); }
	const OperandNode *b_equal(const OperandNode *a) const override { return a->equal(this); }
	const OperandNode *b_notEqual(const OperandNode *a) const override { return a->notEqual(this); }
	const OperandNode *b_and(const OperandNode *a) const override { return a->And(this); }
	const OperandNode *b_or(const OperandNode *a) const override { return a->Or(this); }
	const OperandNode *b_min(const OperandNode *a) const override { return a->min(this); }
	const OperandNode *b_max(const OperandNode *a) const override { return a->max(this); }
	const OperandNode *b_pow(const OperandNode *a) const override { return a->pow(this); }
	const OperandNode *b_fmod(const OperandNode *a) const override { return a->fmod(this); }
	const OperandNode *b_atan2(const OperandNode *a) const override { return a->atan2(this); }
	const OperandNode *b_ldexp(const OperandNode *a) const override { return a->ldexp(this); }
	const OperandNode *b_step(const OperandNode *a) const override { return a->step(this); }
	const OperandNode *b_dot(const OperandNode *a) const override { return a->dot(this); }
	const OperandNode *b_cross(const OperandNode *a) const override { return a->cross(this); }
	const OperandNode *b_distance(const OperandNode *a) const override { return a->distance(this); }
	const OperandNode *b_mulFunc(const OperandNode *a) const override { return a->mulFunc(this); }
	const OperandNode *b_reflect(const OperandNode *a) const override { return a->reflect(this); }

	const OperandNode *clamp(const Vector3Node *b, const Vector3Node *c) const override { return new Vector3Node(XMVectorClamp(v, b->v, c->v)); }
	const OperandNode *b_clamp(const OperandNode *a, const Vector3Node *c) const override { return a->clamp(this, c); }
	const OperandNode *c_clamp(const OperandNode *a, const OperandNode *b) const override { return b->b_clamp(a, this); }

	const OperandNode *lerp(const Vector3Node *b, const Vector3Node *c) const override { return new Vector3Node(XMVectorLerpV(v, b->v, c->v)); }
	const OperandNode *b_lerp(const OperandNode *a, const Vector3Node *c) const override { return a->lerp(this, c); }
	const OperandNode *c_lerp(const OperandNode *a, const OperandNode *b) const override { return b->b_lerp(a, this); }

	const OperandNode *mad(const Vector3Node *b, const Vector3Node *c) const override { return new Vector3Node(XMVectorMultiplyAdd(v, b->v, c->v)); }
	const OperandNode *b_mad(const OperandNode *a, const Vector3Node *c) const override { return a->mad(this, c); }
	const OperandNode *c_mad(const OperandNode *a, const OperandNode *b) const override { return b->b_mad(a, this); }

	const OperandNode *smoothstep(const Vector3Node *b, const Vector3Node *c) const override { XMVECTOR t = XMVectorSaturate(XMVectorDivide(XMVectorSubtract(c->v, v), XMVectorSubtract(b->v, v))); return new Vector3Node(XMVectorMultiply(XMVectorMultiply(t, t), XMVectorNegativeMultiplySubtract(t, XMVectorReplicate(2.0f), XMVectorReplicate(3.0f)))); }
	const OperandNode *b_smoothstep(const OperandNode *a, const Vector3Node *c) const override { return a->smoothstep(this, c); }
	const OperandNode *c_smoothstep(const OperandNode *a, const OperandNode *b) const override { return b->b_smoothstep(a, this); }

	const OperandNode *ifThenElse(const Vector3Node *b, const Vector3Node *c) const override { return new Vector3Node(XMVectorSelect(c->v, b->v, XMVectorNotEqual(v, XMVectorZero()))); }
	const OperandNode *b_ifThenElse(const OperandNode *a, const Vector3Node *c) const override { return a->ifThenElse(this, c); }
	const OperandNode *c_ifThenElse(const OperandNode *a, const OperandNode *b) const override { return b->b_ifThenElse(a, this); }

};

struct Vector2Node : VectorNode
{
	Vector2Node(const XMFLOAT2 &v) : VectorNode(XMLoadFloat2(&v)) {}
	Vector2Node(FXMVECTOR v) : VectorNode(v) {}
	Vector2Node(const ValueNode *rhs) : VectorNode(XMVectorReplicate((float32)rhs->v)) { }

	ExprNode *duplicate() const override { return new Vector2Node(*this); }
	static ExprNode *create(const XMFLOAT2 &v) { return new Vector2Node(v); }

	VectorNode *duplicate(FXMVECTOR v) const override { return new Vector2Node(v); }
	XMFLOAT4 vector() const override { XMFLOAT4 f; XMStoreFloat4(&f, v); f.w = f.z = 0.0f; return f; }
	uint32 ncomp() const override { return 2; }

	const OperandNode *all() const override { return new ValueNode(XMComparisonAllFalse(XMVector2EqualR(v, XMVectorZero())) ? 1.0f : 0.0f); }
	const OperandNode *any() const override { return new ValueNode(XMComparisonAnyFalse(XMVector2EqualR(v, XMVectorZero())) ? 1.0f : 0.0f); }
	const OperandNode *length() const override { return new ValueNode(XMVectorGetX(XMVector2Length(v))); }
	const OperandNode *normalize() const override { return duplicate(XMVector2Normalize(v)); }

	const OperandNode* add(const ValueNode* b) const override { Vector2Node t(b); return add(&t); }
	const OperandNode* sub(const ValueNode* b) const override { Vector2Node t(b); return sub(&t); }
	const OperandNode* mul(const ValueNode* b) const override { Vector2Node t(b); return mul(&t); }
	const OperandNode* div(const ValueNode* b) const override { Vector2Node t(b); return div(&t); }
	const OperandNode* intMod(const ValueNode* b) const override { Vector2Node t(b); return intMod(&t); }
	const OperandNode* less(const ValueNode* b) const override { Vector2Node t(b); return less(&t); }
	const OperandNode* lessEq(const ValueNode* b) const override { Vector2Node t(b); return lessEq(&t); }
	const OperandNode *greater(const ValueNode *b) const override { Vector2Node t(b); return greater(&t); }
	const OperandNode *greaterEq(const ValueNode *b) const override { Vector2Node t(b); return greaterEq(&t); }
	const OperandNode *equal(const ValueNode *b) const override { Vector2Node t(b); return equal(&t); }
	const OperandNode *notEqual(const ValueNode *b) const override { Vector2Node t(b); return notEqual(&t); }
	const OperandNode *And(const ValueNode *b) const override { Vector2Node t(b); return And(&t); }
	const OperandNode *Or(const ValueNode *b) const override { Vector2Node t(b); return Or(&t); }
	const OperandNode *min(const ValueNode *b) const override { Vector2Node t(b); return min(&t); }
	const OperandNode *max(const ValueNode *b) const override { Vector2Node t(b); return max(&t); }
	const OperandNode *pow(const ValueNode *b) const override { Vector2Node t(b); return pow(&t); }
	const OperandNode *fmod(const ValueNode *b) const override { Vector2Node t(b); return fmod(&t); }
	const OperandNode *atan2(const ValueNode *b) const override { Vector2Node t(b); return atan2(&t); }
	const OperandNode *ldexp(const ValueNode *b) const override { Vector2Node t(b); return ldexp(&t); }
	const OperandNode *step(const ValueNode *b) const override { Vector2Node t(b); return step(&t); }
	const OperandNode *dot(const ValueNode *b) const override { Vector2Node t(b); return dot(&t); }
	const OperandNode *distance(const ValueNode *b) const override { Vector2Node t(b); return distance(&t); }
	const OperandNode *mulFunc(const ValueNode *b) const override { Vector2Node t(b); return mulFunc(&t); }
	const OperandNode *reflect(const ValueNode *b) const override { Vector2Node t(b); return reflect(&t); }

	const OperandNode *add(const Vector2Node *b) const override { return VectorNode::add(b); }
	const OperandNode *sub(const Vector2Node *b) const override { return VectorNode::sub(b); }
	const OperandNode *mul(const Vector2Node *b) const override { return VectorNode::mul(b); }
	const OperandNode *div(const Vector2Node *b) const override { return VectorNode::div(b); }
	const OperandNode *intMod(const Vector2Node *b) const override { return VectorNode::intMod(b); }
	const OperandNode *less(const Vector2Node *b) const override { return VectorNode::less(b); }
	const OperandNode *lessEq(const Vector2Node *b) const override { return VectorNode::lessEq(b); }
	const OperandNode *greater(const Vector2Node *b) const override { return VectorNode::greater(b); }
	const OperandNode *greaterEq(const Vector2Node *b) const override { return VectorNode::greaterEq(b); }
	const OperandNode *equal(const Vector2Node *b) const override { return VectorNode::equal(b); }
	const OperandNode *notEqual(const Vector2Node *b) const override { return VectorNode::notEqual(b); }
	const OperandNode *And(const Vector2Node *b) const override { return VectorNode::And(b); }
	const OperandNode *Or(const Vector2Node *b) const override { return VectorNode::Or(b); }
	const OperandNode *min(const Vector2Node *b) const override { return VectorNode::min(b); }
	const OperandNode *max(const Vector2Node *b) const override { return VectorNode::max(b); }
	const OperandNode *pow(const Vector2Node *b) const override { return VectorNode::pow(b); }
	const OperandNode *fmod(const Vector2Node *b) const override { return VectorNode::fmod(b); }
	const OperandNode *atan2(const Vector2Node *b) const override { return VectorNode::atan2(b); }
	const OperandNode *ldexp(const Vector2Node *b) const override { return VectorNode::ldexp(b); }
	const OperandNode *step(const Vector2Node *b) const override { return VectorNode::step(b); }
	const OperandNode *dot(const Vector2Node *b) const override { return new ValueNode(XMVectorGetX(XMVector2Dot(v, b->v))); }
	const OperandNode *distance(const Vector2Node *b) const override { return new ValueNode(XMVectorGetX(XMVector2Length(v - b->v))); }
	const OperandNode *mulFunc(const Vector2Node *b) const override { return mul(b); }
	const OperandNode *reflect(const Vector2Node *b) const override { return duplicate(XMVector2Reflect(v, b->v)); }

	const OperandNode *b_add(const OperandNode *a) const override { return a->add(this); }
	const OperandNode *b_sub(const OperandNode *a) const override { return a->sub(this); }
	const OperandNode *b_mul(const OperandNode *a) const override { return a->mul(this); }
	const OperandNode *b_div(const OperandNode *a) const override { return a->div(this); }
	const OperandNode *b_intMod(const OperandNode *a) const override { return a->intMod(this); }
	const OperandNode *b_less(const OperandNode *a) const override { return a->less(this); }
	const OperandNode *b_lessEq(const OperandNode *a) const override { return a->lessEq(this); }
	const OperandNode *b_greater(const OperandNode *a) const override { return a->greater(this); }
	const OperandNode *b_greaterEq(const OperandNode *a) const override { return a->greaterEq(this); }
	const OperandNode *b_equal(const OperandNode *a) const override { return a->equal(this); }
	const OperandNode *b_notEqual(const OperandNode *a) const override { return a->notEqual(this); }
	const OperandNode *b_and(const OperandNode *a) const override { return a->And(this); }
	const OperandNode *b_or(const OperandNode *a) const override { return a->Or(this); }
	const OperandNode *b_min(const OperandNode *a) const override { return a->min(this); }
	const OperandNode *b_max(const OperandNode *a) const override { return a->max(this); }
	const OperandNode *b_pow(const OperandNode *a) const override { return a->pow(this); }
	const OperandNode *b_fmod(const OperandNode *a) const override { return a->fmod(this); }
	const OperandNode *b_atan2(const OperandNode *a) const override { return a->atan2(this); }
	const OperandNode *b_ldexp(const OperandNode *a) const override { return a->ldexp(this); }
	const OperandNode *b_step(const OperandNode *a) const override { return a->step(this); }
	const OperandNode *b_dot(const OperandNode *a) const override { return a->dot(this); }
	const OperandNode *b_distance(const OperandNode *a) const override { return a->distance(this); }
	const OperandNode *b_mulFunc(const OperandNode *a) const override { return a->mulFunc(this); }
	const OperandNode *b_reflect(const OperandNode *a) const override { return a->reflect(this); }

	const OperandNode *clamp(const Vector2Node *b, const Vector2Node *c) const override { return new Vector2Node(XMVectorClamp(v, b->v, c->v)); }
	const OperandNode *b_clamp(const OperandNode *a, const Vector2Node *c) const override { return a->clamp(this, c); }
	const OperandNode *c_clamp(const OperandNode *a, const OperandNode *b) const override { return b->b_clamp(a, this); }

	const OperandNode *lerp(const Vector2Node *b, const Vector2Node *c) const override { return new Vector2Node(XMVectorLerpV(v, b->v, c->v)); }
	const OperandNode *b_lerp(const OperandNode *a, const Vector2Node *c) const override { return a->lerp(this, c); }
	const OperandNode *c_lerp(const OperandNode *a, const OperandNode *b) const override { return b->b_lerp(a, this); }

	const OperandNode *mad(const Vector2Node *b, const Vector2Node *c) const override { return new Vector2Node(XMVectorMultiplyAdd(v, b->v, c->v)); }
	const OperandNode *b_mad(const OperandNode *a, const Vector2Node *c) const override { return a->mad(this, c); }
	const OperandNode *c_mad(const OperandNode *a, const OperandNode *b) const override { return b->b_mad(a, this); }

	const OperandNode *smoothstep(const Vector2Node *b, const Vector2Node *c) const override { XMVECTOR t = XMVectorSaturate(XMVectorDivide(XMVectorSubtract(c->v, v), XMVectorSubtract(b->v, v))); return new Vector2Node(XMVectorMultiply(XMVectorMultiply(t, t), XMVectorNegativeMultiplySubtract(t, XMVectorReplicate(2.0f), XMVectorReplicate(3.0f)))); }
	const OperandNode *b_smoothstep(const OperandNode *a, const Vector2Node *c) const override { return a->smoothstep(this, c); }
	const OperandNode *c_smoothstep(const OperandNode *a, const OperandNode *b) const override { return b->b_smoothstep(a, this); }

	const OperandNode *ifThenElse(const Vector2Node *b, const Vector2Node *c) const override { return new Vector2Node(XMVectorSelect(c->v, b->v, XMVectorNotEqual(v, XMVectorZero()))); }
	const OperandNode *b_ifThenElse(const OperandNode *a, const Vector2Node *c) const override { return a->ifThenElse(this, c); }
	const OperandNode *c_ifThenElse(const OperandNode *a, const OperandNode *b) const override { return b->b_ifThenElse(a, this); }
};

struct MatrixNode : OperandNode
{
	XMMATRIX v;

	MatrixNode(const MatrixNode &rhs) : v(rhs.v) {}
	MatrixNode(FXMMATRIX v) : v(v) {}

	const OperandNode *operator()() const override { return this; }

	const MatrixNode *asMatrix() const override { return this; }

	virtual MatrixNode *duplicate(FXMMATRIX v) const = 0;
	virtual XMFLOAT4X4 matrix() const = 0;

	const OperandNode *neg() const override;
	const OperandNode *pos() const override;
	const OperandNode *Not() const override;
	const OperandNode *saturate() const override;
	const OperandNode *sin() const override;
	const OperandNode *cos() const override;
	const OperandNode *tan() const override;
	const OperandNode *asin() const override;
	const OperandNode *acos() const override;
	const OperandNode *atan() const override;
	const OperandNode *abs() const override;
	const OperandNode *sqrt() const override;
	const OperandNode *rsqrt() const override;
	const OperandNode *floor() const override;
	const OperandNode *ceil() const override;
	const OperandNode *round() const override;
	const OperandNode *sinh() const override;
	const OperandNode *cosh() const override;
	const OperandNode *tanh() const override;
	const OperandNode *log10() const override;
	const OperandNode *log() const override;
	const OperandNode *log2() const override;
	const OperandNode *exp() const override;
	const OperandNode *exp2() const override;
	const OperandNode *frac() const override;
	const OperandNode *sign() const override;
	const OperandNode *rad() const override;
	const OperandNode *deg() const override;
	const OperandNode *isNaN() const override;
	const OperandNode *isInf() const override;
	const OperandNode *trunc() const override;

	const OperandNode *add(const MatrixNode *b) const;
	const OperandNode *sub(const MatrixNode *b) const;
	const OperandNode *mul(const MatrixNode *b) const;
	const OperandNode *div(const MatrixNode *b) const;
	const OperandNode *intMod(const MatrixNode *b) const;
	const OperandNode *less(const MatrixNode *b) const;
	const OperandNode *lessEq(const MatrixNode *b) const;
	const OperandNode *greater(const MatrixNode *b) const;
	const OperandNode *greaterEq(const MatrixNode *b) const;
	const OperandNode *equal(const MatrixNode *b) const;
	const OperandNode *notEqual(const MatrixNode *b) const;
	const OperandNode *And(const MatrixNode *b) const;
	const OperandNode *Or(const MatrixNode *b) const;
	const OperandNode *min(const MatrixNode *b) const;
	const OperandNode *max(const MatrixNode *b) const;
	const OperandNode *pow(const MatrixNode *b) const;
	const OperandNode *fmod(const MatrixNode *b) const;
	const OperandNode *atan2(const MatrixNode *b) const;
	const OperandNode *ldexp(const MatrixNode *b) const;
	const OperandNode *step(const MatrixNode *b) const;

};

struct Matrix44Node : MatrixNode
{
	Matrix44Node(const XMFLOAT4X4 &v) : MatrixNode(XMLoadFloat4x4(&v)) {}
	Matrix44Node(FXMMATRIX v) : MatrixNode(v) {}
	Matrix44Node(const ValueNode *rhs) : Matrix44Node(XMFLOAT4X4((float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v, (float32)rhs->v)) { }

	ExprNode *duplicate() const override { return new Matrix44Node(*this); }
	static ExprNode *create(const XMFLOAT4X4 &v) { return new Matrix44Node(v); }

	MatrixNode *duplicate(FXMMATRIX v) const override { return new Matrix44Node(v); }
	XMFLOAT4X4 matrix() const override { XMFLOAT4X4 f; XMStoreFloat4x4(&f, v); return f; }

	const OperandNode *all() const override;
	const OperandNode *any() const override;
	const OperandNode *determinant() const override;
	const OperandNode *inverse() const override;
	const OperandNode *transpose() const override;

	const OperandNode *add(const ValueNode *b) const override;
	const OperandNode *sub(const ValueNode *b) const override;
	const OperandNode *mul(const ValueNode *b) const override;
	const OperandNode *div(const ValueNode *b) const override;
	const OperandNode *intMod(const ValueNode *b) const override;
	const OperandNode *less(const ValueNode *b) const override;
	const OperandNode *lessEq(const ValueNode *b) const override;
	const OperandNode *greater(const ValueNode *b) const override;
	const OperandNode *greaterEq(const ValueNode *b) const override;
	const OperandNode *equal(const ValueNode *b) const override;
	const OperandNode *notEqual(const ValueNode *b) const override;
	const OperandNode *And(const ValueNode *b) const override;
	const OperandNode *Or(const ValueNode *b) const override;
	const OperandNode *min(const ValueNode *b) const override;
	const OperandNode *max(const ValueNode *b) const override;
	const OperandNode *pow(const ValueNode *b) const override;
	const OperandNode *fmod(const ValueNode *b) const override;
	const OperandNode *atan2(const ValueNode *b) const override;
	const OperandNode *ldexp(const ValueNode *b) const override;
	const OperandNode *step(const ValueNode *b) const override;
	const OperandNode *mulFunc(const ValueNode *b) const override;
	const OperandNode *subscript(const ValueNode *b) const override;

	const OperandNode *add(const Matrix44Node *b) const override { return MatrixNode::add(b); }
	const OperandNode *sub(const Matrix44Node *b) const override { return MatrixNode::sub(b); }
	const OperandNode *mul(const Matrix44Node *b) const override { return MatrixNode::mul(b); }
	const OperandNode *div(const Matrix44Node *b) const override { return MatrixNode::div(b); }
	const OperandNode *intMod(const Matrix44Node *b) const override { return MatrixNode::intMod(b); }
	const OperandNode *less(const Matrix44Node *b) const override { return MatrixNode::less(b); }
	const OperandNode *lessEq(const Matrix44Node *b) const override { return MatrixNode::lessEq(b); }
	const OperandNode *greater(const Matrix44Node *b) const override { return MatrixNode::greater(b); }
	const OperandNode *greaterEq(const Matrix44Node *b) const override { return MatrixNode::greaterEq(b); }
	const OperandNode *equal(const Matrix44Node *b) const override { return MatrixNode::equal(b); }
	const OperandNode *notEqual(const Matrix44Node *b) const override { return MatrixNode::notEqual(b); }
	const OperandNode *And(const Matrix44Node *b) const override { return MatrixNode::And(b); }
	const OperandNode *Or(const Matrix44Node *b) const override { return MatrixNode::Or(b); }
	const OperandNode *min(const Matrix44Node *b) const override { return MatrixNode::min(b); }
	const OperandNode *max(const Matrix44Node *b) const override { return MatrixNode::max(b); }
	const OperandNode *pow(const Matrix44Node *b) const override { return MatrixNode::pow(b); }
	const OperandNode *fmod(const Matrix44Node *b) const override { return MatrixNode::fmod(b); }
	const OperandNode *atan2(const Matrix44Node *b) const override { return MatrixNode::atan2(b); }
	const OperandNode *ldexp(const Matrix44Node *b) const override { return MatrixNode::ldexp(b); }
	const OperandNode *step(const Matrix44Node *b) const override { return MatrixNode::step(b); }
	const OperandNode *mulFunc(const Matrix44Node *b) const override;
	
	const OperandNode *mul(const Vector4Node *b) const override;
	const OperandNode *mulFunc(const Vector4Node *b) const override;

	const OperandNode *b_add(const OperandNode *a) const override { return a->add(this); }
	const OperandNode *b_sub(const OperandNode *a) const override { return a->sub(this); }
	const OperandNode *b_mul(const OperandNode *a) const override { return a->mul(this); }
	const OperandNode *b_div(const OperandNode *a) const override { return a->div(this); }
	const OperandNode *b_intMod(const OperandNode *a) const override { return a->intMod(this); }
	const OperandNode *b_less(const OperandNode *a) const override { return a->less(this); }
	const OperandNode *b_lessEq(const OperandNode *a) const override { return a->lessEq(this); }
	const OperandNode *b_greater(const OperandNode *a) const override { return a->greater(this); }
	const OperandNode *b_greaterEq(const OperandNode *a) const override { return a->greaterEq(this); }
	const OperandNode *b_equal(const OperandNode *a) const override { return a->equal(this); }
	const OperandNode *b_notEqual(const OperandNode *a) const override { return a->notEqual(this); }
	const OperandNode *b_and(const OperandNode *a) const override { return a->And(this); }
	const OperandNode *b_or(const OperandNode *a) const override { return a->Or(this); }
	const OperandNode *b_min(const OperandNode *a) const override { return a->min(this); }
	const OperandNode *b_max(const OperandNode *a) const override { return a->max(this); }
	const OperandNode *b_pow(const OperandNode *a) const override { return a->pow(this); }
	const OperandNode *b_fmod(const OperandNode *a) const override { return a->fmod(this); }
	const OperandNode *b_atan2(const OperandNode *a) const override { return a->atan2(this); }
	const OperandNode *b_ldexp(const OperandNode *a) const override { return a->ldexp(this); }
	const OperandNode *b_step(const OperandNode *a) const override { return a->step(this); }
	const OperandNode *b_mulFunc(const OperandNode *a) const override { return a->mulFunc(this); }

	const OperandNode *clamp(const Matrix44Node *b, const Matrix44Node *c) const override { return new Matrix44Node(XMMATRIX(XMVectorClamp(v.r[0], b->v.r[0], c->v.r[0]), XMVectorClamp(v.r[1], b->v.r[1], c->v.r[1]), XMVectorClamp(v.r[2], b->v.r[2], c->v.r[2]), XMVectorClamp(v.r[3], b->v.r[3], c->v.r[3]))); }
	const OperandNode *b_clamp(const OperandNode *a, const Matrix44Node *c) const override { return a->clamp(this, c); }
	const OperandNode *c_clamp(const OperandNode *a, const OperandNode *b) const override { return b->b_clamp(a, this); }

	const OperandNode *lerp(const Matrix44Node *b, const Matrix44Node *c) const override { return new Matrix44Node(XMMATRIX(XMVectorLerpV(v.r[0], b->v.r[0], c->v.r[0]), XMVectorLerpV(v.r[1], b->v.r[1], c->v.r[1]), XMVectorLerpV(v.r[2], b->v.r[2], c->v.r[2]), XMVectorLerpV(v.r[3], b->v.r[3], c->v.r[3]))); }
	const OperandNode *b_lerp(const OperandNode *a, const Matrix44Node *c) const override { return a->lerp(this, c); }
	const OperandNode *c_lerp(const OperandNode *a, const OperandNode *b) const override { return b->b_lerp(a, this); }

	const OperandNode *mad(const Matrix44Node *b, const Matrix44Node *c) const override { return new Matrix44Node(XMMATRIX(XMVectorMultiplyAdd(v.r[0], b->v.r[0], c->v.r[0]), XMVectorMultiplyAdd(v.r[1], b->v.r[1], c->v.r[1]), XMVectorMultiplyAdd(v.r[2], b->v.r[2], c->v.r[2]), XMVectorMultiplyAdd(v.r[3], b->v.r[3], c->v.r[3]))); }
	const OperandNode *b_mad(const OperandNode *a, const Matrix44Node *c) const override { return a->mad(this, c); }
	const OperandNode *c_mad(const OperandNode *a, const OperandNode *b) const override { return b->b_mad(a, this); }

};












struct OperatorNode : ExprNode
{
	mutable std::unique_ptr<const OperandNode> r;
	const OperandNode *result(const OperandNode *Or) const
	{
		r.reset(Or);
		return Or;
	}

	bool isOperator() const override { return true; }
	virtual uint32 pcount() const = 0;
};


struct NullaryOperatorNode : OperatorNode
{
	NullaryOperatorNode() {}
	NullaryOperatorNode(const NullaryOperatorNode &rhs) {}

	uint32 pcount() const override { return 0; }

	bool build(ExprNodePtrVector &v) override
	{
		v.pop_back();
		return true;
	}
};

struct UnaryOperatorNode : OperatorNode
{
	ExprNodePtr a;

	UnaryOperatorNode() {}
	UnaryOperatorNode(const UnaryOperatorNode &n) : a(n.a->duplicate()) {}

	uint32 pcount() const override { return 1; }

	bool build(ExprNodePtrVector &v) override
	{
		v.pop_back();
		if (v.empty())
			return false;
		a = std::move(v.back());
		return a->build(v);
	}
};


struct BinaryOperatorNode : OperatorNode
{
	ExprNodePtr a, b;


	BinaryOperatorNode() {}
	BinaryOperatorNode(const BinaryOperatorNode &n) : a(n.a->duplicate()), b(n.b->duplicate()) {}

	uint32 pcount() const override { return 2; }

	bool build(ExprNodePtrVector &v) override
	{
		v.pop_back();
		if (v.empty())
			return false;
		b = std::move(v.back());
		if (!b->build(v))
			return false;
		if (v.empty())
			return false;
		a = std::move(v.back());
		return a->build(v);
	}
};

struct TernaryOperatorNode : OperatorNode
{
	ExprNodePtr a, b, c;

	TernaryOperatorNode()  {}
	TernaryOperatorNode(const TernaryOperatorNode &n) : a(n.a->duplicate()), b(n.b->duplicate()), c(n.c->duplicate()) {}

	uint32 pcount() const override { return 3; }

	bool build(ExprNodePtrVector &v) override
	{
		v.pop_back();
		if (v.empty())
			return false;
		c = std::move(v.back());
		if (!c->build(v))
			return false;
		if (v.empty())
			return false;
		b = std::move(v.back());
		if (!b->build(v))
			return false;
		if (v.empty())
			return false;
		a = std::move(v.back());
		return a->build(v);
	}
};

struct NnaryOperatorNode : OperatorNode
{
	ExprNodePtrVector n;

	NnaryOperatorNode(uint32 pcount) : n(pcount) {}
	NnaryOperatorNode(const NnaryOperatorNode &n) 
	{
		for (size_t i = 0; i < n.n.size(); i++)
			this->n.push_back(ExprNodePtr(n.n[i]->duplicate()));
	}

	uint32 pcount() const override { return (uint32) n.size(); }

	bool build(ExprNodePtrVector &v) override
	{
		v.pop_back();
		for (size_t i = 0; i < n.size(); i++) {
			if (v.empty())
				return false;
			n[i] = std::move(v.back());
			if (!n[i]->build(v))
				return false;
		}
		return true;
	}
};

struct NegOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->neg()); }
	ExprNode *duplicate() const override { return new NegOperatorNode(*this); }
	static ExprNode *create() { return new NegOperatorNode(); }
};

struct PosOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->pos()); }
	ExprNode *duplicate() const override { return new PosOperatorNode(*this); }
	static ExprNode *create() { return new PosOperatorNode(); }
};

struct NotOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->Not()); }
	ExprNode *duplicate() const override { return new NotOperatorNode(*this); }
	static ExprNode *create() { return new NotOperatorNode(); }
};

struct BWNotOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->bwNot()); }
	ExprNode *duplicate() const override { return new BWNotOperatorNode(*this); }
	static ExprNode *create() { return new BWNotOperatorNode(); }
};

struct SaturateOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->saturate()); }
	ExprNode *duplicate() const override { return new SaturateOperatorNode(*this); }
	static ExprNode *create() { return new SaturateOperatorNode(); }
};

struct SinOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->sin()); }
	ExprNode *duplicate() const override { return new SinOperatorNode(*this); }
	static ExprNode *create() { return new SinOperatorNode(); }
};

struct CosOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->cos()); }
	ExprNode *duplicate() const override { return new CosOperatorNode(*this); }
	static ExprNode *create() { return new CosOperatorNode(); }
};

struct TanOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->tan()); }
	ExprNode *duplicate() const override { return new TanOperatorNode(*this); }
	static ExprNode *create() { return new TanOperatorNode(); }
};

struct aSinOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->asin()); }
	ExprNode *duplicate() const override { return new aSinOperatorNode(*this); }
	static ExprNode *create() { return new aSinOperatorNode(); }
};

struct aCosOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->acos()); }
	ExprNode *duplicate() const override { return new aCosOperatorNode(*this); }
	static ExprNode *create() { return new aCosOperatorNode(); }
};

struct aTanOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->atan()); }
	ExprNode *duplicate() const override { return new aTanOperatorNode(*this); }
	static ExprNode *create() { return new aTanOperatorNode(); }
};

struct AbsOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->abs()); }
	ExprNode *duplicate() const override { return new AbsOperatorNode(*this); }
	static ExprNode *create() { return new AbsOperatorNode(); }
};

struct SqrtOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->sqrt()); }
	ExprNode *duplicate() const override { return new SqrtOperatorNode(*this); }
	static ExprNode *create() { return new SqrtOperatorNode(); }
};

struct RSqrtOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->rsqrt()); }
	ExprNode *duplicate() const override { return new RSqrtOperatorNode(*this); }
	static ExprNode *create() { return new RSqrtOperatorNode(); }
};

struct FloorOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->floor()); }
	ExprNode *duplicate() const override { return new FloorOperatorNode(*this); }
	static ExprNode *create() { return new FloorOperatorNode(); }
};

struct CeilOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->ceil()); }
	ExprNode *duplicate() const override { return new CeilOperatorNode(*this); }
	static ExprNode *create() { return new CeilOperatorNode(); }
};

struct RoundOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->round()); }
	ExprNode *duplicate() const override { return new RoundOperatorNode(*this); }
	static ExprNode *create() { return new RoundOperatorNode(); }
};

struct SinhOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->sinh()); }
	ExprNode *duplicate() const override { return new SinhOperatorNode(*this); }
	static ExprNode *create() { return new SinhOperatorNode(); }
};

struct CoshOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->cosh()); }
	ExprNode *duplicate() const override { return new CoshOperatorNode(*this); }
	static ExprNode *create() { return new CoshOperatorNode(); }
};

struct TanhOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->tanh()); }
	ExprNode *duplicate() const override { return new TanhOperatorNode(*this); }
	static ExprNode *create() { return new TanhOperatorNode(); }
};

struct LogOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->log()); }
	ExprNode *duplicate() const override { return new LogOperatorNode(*this); }
	static ExprNode *create() { return new LogOperatorNode(); }
};

struct Log10OperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->log10()); }
	ExprNode *duplicate() const override { return new Log10OperatorNode(*this); }
	static ExprNode *create() { return new Log10OperatorNode(); }
};

struct Log2OperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->log2()); }
	ExprNode *duplicate() const override { return new Log2OperatorNode(*this); }
	static ExprNode *create() { return new Log2OperatorNode(); }
};

struct ExpOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->exp()); }
	ExprNode *duplicate() const override { return new ExpOperatorNode(*this); }
	static ExprNode *create() { return new ExpOperatorNode(); }
};

struct Exp2OperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->exp2()); }
	ExprNode *duplicate() const override { return new Exp2OperatorNode(*this); }
	static ExprNode *create() { return new Exp2OperatorNode(); }
};

struct FracOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->frac()); }
	ExprNode *duplicate() const override { return new FracOperatorNode(*this); }
	static ExprNode *create() { return new FracOperatorNode(); }
};

struct SignOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->sign()); }
	ExprNode *duplicate() const override { return new SignOperatorNode(*this); }
	static ExprNode *create() { return new SignOperatorNode(); }
};

struct RadOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->rad()); }
	ExprNode *duplicate() const override { return new RadOperatorNode(*this); }
	static ExprNode *create() { return new RadOperatorNode(); }
};

struct DegOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->deg()); }
	ExprNode *duplicate() const override { return new DegOperatorNode(*this); }
	static ExprNode *create() { return new DegOperatorNode(); }
};

struct IsNaNOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->isNaN()); }
	ExprNode *duplicate() const override { return new IsNaNOperatorNode(*this); }
	static ExprNode *create() { return new IsNaNOperatorNode(); }
};

struct IsInfOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->isInf()); }
	ExprNode *duplicate() const override { return new IsInfOperatorNode(*this); }
	static ExprNode *create() { return new IsInfOperatorNode(); }
};

struct TruncOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->trunc()); }
	ExprNode *duplicate() const override { return new TruncOperatorNode(*this); }
	static ExprNode *create() { return new TruncOperatorNode(); }
};

struct AllOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->all()); }
	ExprNode *duplicate() const override { return new AllOperatorNode(*this); }
	static ExprNode *create() { return new AllOperatorNode(); }
};

struct AnyOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->any()); }
	ExprNode *duplicate() const override { return new AnyOperatorNode(*this); }
	static ExprNode *create() { return new AnyOperatorNode(); }
};

struct LengthOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->length()); }
	ExprNode *duplicate() const override { return new LengthOperatorNode(*this); }
	static ExprNode *create() { return new LengthOperatorNode(); }
};

struct NormalizeOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->normalize()); }
	ExprNode *duplicate() const override { return new NormalizeOperatorNode(*this); }
	static ExprNode *create() { return new NormalizeOperatorNode(); }
};

struct DeterminantOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->determinant()); }
	ExprNode *duplicate() const override { return new DeterminantOperatorNode(*this); }
	static ExprNode *create() { return new DeterminantOperatorNode(); }
};

struct TransposeOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->transpose()); }
	ExprNode *duplicate() const override { return new TransposeOperatorNode(*this); }
	static ExprNode *create() { return new TransposeOperatorNode(); }
};

struct InverseOperatorNode : UnaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*a)()->inverse()); }
	ExprNode *duplicate() const override { return new InverseOperatorNode(*this); }
	static ExprNode *create() { return new InverseOperatorNode(); }
};




struct AddOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_add((*a)())); }
	ExprNode *duplicate() const override { return new AddOperatorNode(*this); }
	static ExprNode *create() { return new AddOperatorNode(); }
};

struct SubOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_sub((*a)())); }
	ExprNode *duplicate() const override { return new SubOperatorNode(*this); }
	static ExprNode *create() { return new SubOperatorNode(); }
};

struct MulOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_mul((*a)())); }
	ExprNode *duplicate() const override { return new MulOperatorNode(*this); }
	static ExprNode *create() { return new MulOperatorNode(); }
};

struct DivOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_div((*a)())); }
	ExprNode *duplicate() const override { return new DivOperatorNode(*this); }
	static ExprNode *create() { return new DivOperatorNode(); }
};

struct PowOprOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const { result((*b)()->b_pow((*a)())); }
	ExprNode *duplicate() const override { return new PowOprOperatorNode(*this); }
	static ExprNode *create() { return new PowOprOperatorNode(); }
};

struct IntModOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_intMod((*a)())); }
	ExprNode *duplicate() const override { return new IntModOperatorNode(*this); }
	static ExprNode *create() { return new IntModOperatorNode(); }
};

struct LessOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_less((*a)())); }
	ExprNode *duplicate() const override { return new LessOperatorNode(*this); }
	static ExprNode *create() { return new LessOperatorNode(); }
};

struct LessEqOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_lessEq((*a)())); }
	ExprNode *duplicate() const override { return new LessEqOperatorNode(*this); }
	static ExprNode *create() { return new LessEqOperatorNode(); }
};

struct GreaterOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_greater((*a)())); }
	ExprNode *duplicate() const override { return new GreaterOperatorNode(*this); }
	static ExprNode *create() { return new GreaterOperatorNode(); }
};

struct GreaterEqOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_greaterEq((*a)())); }
	ExprNode *duplicate() const override { return new GreaterEqOperatorNode(*this); }
	static ExprNode *create() { return new GreaterEqOperatorNode(); }
};

struct EqOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_equal((*a)())); }
	ExprNode *duplicate() const override { return new EqOperatorNode(*this); }
	static ExprNode *create() { return new EqOperatorNode(); }
};

struct NotEqOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_notEqual((*a)())); }
	ExprNode *duplicate() const override { return new NotEqOperatorNode(*this); }
	static ExprNode *create() { return new NotEqOperatorNode(); }
};

struct AndOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_and((*a)())); }
	ExprNode *duplicate() const override { return new AndOperatorNode(*this); }
	static ExprNode *create() { return new AndOperatorNode(); }
};

struct OrOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_or((*a)())); }
	ExprNode *duplicate() const override { return new OrOperatorNode(*this); }
	static ExprNode *create() { return new OrOperatorNode(); }
};

struct MinOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_min((*a)())); }
	ExprNode *duplicate() const override { return new MinOperatorNode(*this); }
	static ExprNode *create() { return new MinOperatorNode(); }
};

struct MaxOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_max((*a)())); }
	ExprNode *duplicate() const override { return new MaxOperatorNode(*this); }
	static ExprNode *create() { return new MaxOperatorNode(); }
};

struct PowOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_pow((*a)())); }
	ExprNode *duplicate() const override { return new PowOperatorNode(*this); }
	static ExprNode *create() { return new PowOperatorNode(); }
};

struct FmodOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_fmod((*a)())); }
	ExprNode *duplicate() const override { return new FmodOperatorNode(*this); }
	static ExprNode *create() { return new FmodOperatorNode(); }
};

struct ShiftLeftOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_bwShiftLeft((*a)())); }
	ExprNode *duplicate() const override { return new ShiftLeftOperatorNode(*this); }
	static ExprNode *create() { return new ShiftLeftOperatorNode(); }
};

struct ShiftRightOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_bwShiftRight((*a)())); }
	ExprNode *duplicate() const override { return new ShiftRightOperatorNode(*this); }
	static ExprNode *create() { return new ShiftRightOperatorNode(); }
};

struct BWAndOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_bwAnd((*a)())); }
	ExprNode *duplicate() const override { return new BWAndOperatorNode(*this); }
	static ExprNode *create() { return new BWAndOperatorNode(); }
};

struct BWOrOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_bwOr((*a)())); }
	ExprNode *duplicate() const override { return new BWOrOperatorNode(*this); }
	static ExprNode *create() { return new BWOrOperatorNode(); }
};

struct BWXOrOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_bwXor((*a)())); }
	ExprNode *duplicate() const override { return new BWXOrOperatorNode(*this); }
	static ExprNode *create() { return new BWXOrOperatorNode(); }
};
struct ATan2OperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_atan2((*a)())); }
	ExprNode *duplicate() const override { return new ATan2OperatorNode(*this); }
	static ExprNode *create() { return new ATan2OperatorNode(); }
};

struct LDExpOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_ldexp((*a)())); }
	ExprNode *duplicate() const override { return new LDExpOperatorNode(*this); }
	static ExprNode *create() { return new LDExpOperatorNode(); }
};

struct StepOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_step((*a)())); }
	ExprNode *duplicate() const override { return new StepOperatorNode(*this); }
	static ExprNode *create() { return new StepOperatorNode(); }
};

struct DotOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_dot((*a)())); }
	ExprNode *duplicate() const override { return new DotOperatorNode(*this); }
	static ExprNode *create() { return new DotOperatorNode(); }
};

struct CrossOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_cross((*a)())); }
	ExprNode *duplicate() const override { return new CrossOperatorNode(*this); }
	static ExprNode *create() { return new CrossOperatorNode(); }
};

struct MulFuncOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_mulFunc((*a)())); }
	ExprNode *duplicate() const override { return new MulFuncOperatorNode(*this); }
	static ExprNode *create() { return new MulFuncOperatorNode(); }
};

struct DistanceOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_distance((*a)())); }
	ExprNode *duplicate() const override { return new DistanceOperatorNode(*this); }
	static ExprNode *create() { return new DistanceOperatorNode(); }
};

struct ReflectOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_reflect((*a)())); }
	ExprNode *duplicate() const override { return new ReflectOperatorNode(*this); }
	static ExprNode *create() { return new ReflectOperatorNode(); }
};

struct ClampOperatorNode : TernaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*c)()->c_clamp((*a)(), (*b)())); }
	ExprNode *duplicate() const override { return new ClampOperatorNode(*this); }
	static ExprNode *create() { return new ClampOperatorNode(); }
};

struct LerpOperatorNode : TernaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*c)()->c_lerp((*a)(), (*b)())); }
	ExprNode *duplicate() const override { return new LerpOperatorNode(*this); }
	static ExprNode *create() { return new LerpOperatorNode(); }
};

struct LitOperatorNode : TernaryOperatorNode
{
	const OperandNode *operator()() const override { throw ExprNotImplException(); }
	ExprNode *duplicate() const override { return new LitOperatorNode(*this); }
	static ExprNode *create() { return new LitOperatorNode(); }
};

struct MadOperatorNode : TernaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*c)()->c_mad((*a)(), (*b)())); }
	ExprNode *duplicate() const override { return new MadOperatorNode(*this); }
	static ExprNode *create() { return new MadOperatorNode(); }
};

struct SmoothstepOperatorNode : TernaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*c)()->c_smoothstep((*a)(), (*b)())); }
	ExprNode *duplicate() const override { return new SmoothstepOperatorNode(*this); }
	static ExprNode *create() { return new SmoothstepOperatorNode(); }
};


struct RefractOperatorNode : TernaryOperatorNode
{
	const OperandNode *operator()() const override { throw ExprNotImplException(); }
	ExprNode *duplicate() const override { return new RefractOperatorNode(*this); }
	static ExprNode *create() { return new RefractOperatorNode(); }
};

struct IfThenElseOperatorNode : TernaryOperatorNode
{
	const OperandNode *operator()() const override 
	{ 
		const OperandNode *aon = (*a)();
		if (aon->asValue() != nullptr)
			return aon->asValue()->v != 0.0 ? (*b)() : (*c)();
		return result((*c)()->c_ifThenElse(aon, (*b)())); 
	}
	ExprNode *duplicate() const override { return new IfThenElseOperatorNode(*this); }
	static ExprNode *create() { return new IfThenElseOperatorNode(); }
};


struct RandOperatorNode : NullaryOperatorNode
{
	const OperandNode *operator()() const override { return result(new ValueNode((float64)rand() / (float64)RAND_MAX)); }
	ExprNode *duplicate() const override { return new RandOperatorNode(*this); }
	static ExprNode *create() { return new RandOperatorNode(); }
};

struct CallbackOperatorNode : NullaryOperatorNode
{
	uint32 index;
	ExpressionCallback *cb;
	CallbackOperatorNode(uint32 index, ExpressionCallback *cb) : index(index), cb(cb) {}

	const OperandNode *operator()() const override { return result(cb->GetOperand(index)); }
	ExprNode *duplicate() const override { return new CallbackOperatorNode(index, cb); }
	static ExprNode *create(uint32 index, ExpressionCallback *cb) { return new CallbackOperatorNode(index, cb); }
};

struct SwizzleOperatorNode : UnaryOperatorNode
{
	uint32 mask;
	SwizzleOperatorNode(uint32 mask) : mask(mask) {}

	const OperandNode *operator()() const override { return result((*a)()->swizzle(mask)); }
	ExprNode *duplicate() const override { return new SwizzleOperatorNode(mask); }
	static ExprNode *create(uint32 mask) { return new SwizzleOperatorNode(mask); }
};

struct SubscriptOperatorNode : BinaryOperatorNode
{
	const OperandNode *operator()() const override { return result((*b)()->b_subscript((*a)())); }
	ExprNode *duplicate() const override { return new SubscriptOperatorNode(*this); }
	static ExprNode *create() { return new SubscriptOperatorNode(); }
};

struct VectorConstructorOperatorNode : NnaryOperatorNode
{
	uint32 ccount;
	VectorConstructorOperatorNode(uint32 ccount, uint32 pcount) : NnaryOperatorNode(pcount), ccount(ccount) {}
	const OperandNode *operator()() const override;
	ExprNode *duplicate() const override { return new VectorConstructorOperatorNode(ccount, pcount()); }
	static ExprNode *create(uint32 ccount, uint32 pcount) { return new VectorConstructorOperatorNode(ccount, pcount); }
};


}
}