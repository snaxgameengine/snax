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

#include "pch.h"
#include "ExpressionParserTree.h"
#include <DirectXMath.h>

using namespace m3d;
using namespace m3d::expr;

constexpr float32 TODEG = 180.0f / XM_PI;
constexpr float32 TORAD = XM_PI / 180.0f;

const OperandNode *ValueNode::neg() const { return new ValueNode(-v); }
const OperandNode *ValueNode::pos() const { return new ValueNode(v); }
const OperandNode *ValueNode::Not() const { return new ValueNode(v == 0.0 ? 1.0 : 0.0); }
const OperandNode *ValueNode::bwNot() const { return new ValueNode((float64)(~(uint32)v)); }
const OperandNode *ValueNode::saturate() const { return new ValueNode(v < 0.0 ? 0.0 : (v > 1.0 ? 1.0 : v)); }
const OperandNode *ValueNode::sin() const { return new ValueNode(::sin(v)); }
const OperandNode *ValueNode::cos() const { return new ValueNode(::cos(v)); }
const OperandNode *ValueNode::tan() const { return new ValueNode(::tan(v)); }
const OperandNode *ValueNode::asin() const { return new ValueNode(::asin(v)); }
const OperandNode *ValueNode::acos() const { return new ValueNode(::acos(v)); }
const OperandNode *ValueNode::atan() const { return new ValueNode(::atan(v)); }
const OperandNode *ValueNode::abs() const { return new ValueNode(::abs(v)); }
const OperandNode *ValueNode::sqrt() const { return new ValueNode(::sqrt(v)); }
const OperandNode *ValueNode::rsqrt() const { return new ValueNode(1.0 / ::sqrt(v)); }
const OperandNode *ValueNode::floor() const { return new ValueNode(::floor(v)); }
const OperandNode *ValueNode::ceil() const { return new ValueNode(::ceil(v)); }
const OperandNode *ValueNode::round() const { return new ValueNode(::floor(v + 0.5)); }
const OperandNode *ValueNode::sinh() const { return new ValueNode(::sinh(v)); }
const OperandNode *ValueNode::cosh() const { return new ValueNode(::cosh(v)); }
const OperandNode *ValueNode::tanh() const { return new ValueNode(::tanh(v)); }
const OperandNode *ValueNode::log() const { return new ValueNode(::log(v)); }
const OperandNode *ValueNode::log10() const { return new ValueNode(::log10(v)); }
const OperandNode *ValueNode::log2() const { return new ValueNode(::log2(v)); }
const OperandNode *ValueNode::exp() const { return new ValueNode(::exp(v)); }
const OperandNode *ValueNode::exp2() const { return new ValueNode(::exp2(v)); }
const OperandNode *ValueNode::frac() const { return new ValueNode(v - ::floor(v)); }
const OperandNode *ValueNode::sign() const { return new ValueNode(v < 0.0 ? -1.0 : (v > 0.0 ? 1.0 : 0.0)); }
const OperandNode *ValueNode::rad() const { return new ValueNode(v * TORAD); }
const OperandNode *ValueNode::deg() const { return new ValueNode(v * TODEG); }
const OperandNode *ValueNode::isNaN() const { return new ValueNode(v != v ? 1.0 : 0.0); }
const OperandNode *ValueNode::isInf() const { return new ValueNode(v == std::numeric_limits<float64>::infinity() ? 1.0f : (v == -std::numeric_limits<float64>::infinity() ? -1.0f : 0.0f)); }
const OperandNode *ValueNode::trunc() const { return new ValueNode(::trunc(v)); }

const OperandNode *ValueNode::add(const ValueNode *b) const { return new ValueNode(v + b->v); }
const OperandNode *ValueNode::sub(const ValueNode *b) const { return new ValueNode(v - b->v); }
const OperandNode *ValueNode::mul(const ValueNode *b) const { return new ValueNode(v * b->v); }
const OperandNode *ValueNode::div(const ValueNode *b) const { return new ValueNode(v / b->v); }
const OperandNode *ValueNode::intMod(const ValueNode *b) const { return new ValueNode(float64(int(v) % int(b->v))); }
const OperandNode *ValueNode::less(const ValueNode *b) const { return new ValueNode(v < b->v); }
const OperandNode *ValueNode::lessEq(const ValueNode *b) const { return new ValueNode(v <= b->v); }
const OperandNode *ValueNode::greater(const ValueNode *b) const { return new ValueNode(v > b->v); }
const OperandNode *ValueNode::greaterEq(const ValueNode *b) const { return new ValueNode(v >= b->v); }
const OperandNode *ValueNode::equal(const ValueNode *b) const { return new ValueNode(v == b->v); }
const OperandNode *ValueNode::notEqual(const ValueNode *b) const { return new ValueNode(v != b->v); }
const OperandNode *ValueNode::And(const ValueNode *b) const { return new ValueNode(v != 0.0 && b->v != 0.0 ? 1.0 : 0.0); }
const OperandNode *ValueNode::Or(const ValueNode *b) const { return new ValueNode(v != 0.0 || b->v != 0.0 ? 1.0 : 0.0); }
const OperandNode *ValueNode::min(const ValueNode *b) const { return new ValueNode(std::min(v, b->v)); }
const OperandNode *ValueNode::max(const ValueNode *b) const { return new ValueNode(std::max(v, b->v)); }
const OperandNode *ValueNode::pow(const ValueNode *b) const { return new ValueNode(::pow(v, b->v)); }
const OperandNode *ValueNode::fmod(const ValueNode *b) const { return new ValueNode(::fmod(v, b->v)); }
const OperandNode *ValueNode::bwShiftLeft(const ValueNode *b) const { return new ValueNode(uint32(v) << uint32(b->v)); }
const OperandNode *ValueNode::bwShiftRight(const ValueNode *b) const { return new ValueNode(uint32(v) >> uint32(b->v)); }
const OperandNode *ValueNode::bwAnd(const ValueNode *b) const { return new ValueNode(uint32(v) & uint32(b->v)); }
const OperandNode *ValueNode::bwOr(const ValueNode *b) const { return new ValueNode(uint32(v) | uint32(b->v)); }
const OperandNode *ValueNode::bwXor(const ValueNode *b) const { return new ValueNode(uint32(v) ^ uint32(b->v)); }
const OperandNode *ValueNode::atan2(const ValueNode *b) const { return new ValueNode(::atan2(v, b->v)); }
const OperandNode *ValueNode::ldexp(const ValueNode *b) const { return new ValueNode(::ldexp(v, b->v)); }
const OperandNode *ValueNode::step(const ValueNode *b) const { return new ValueNode(v >= b->v ? 1.0 : 0.0); }
const OperandNode *ValueNode::mulFunc(const ValueNode *b) const { return mul(b); }

const OperandNode *ValueNode::add(const Vector4Node *b) const { return Vector4Node(this).add(b); }
const OperandNode *ValueNode::sub(const Vector4Node *b) const { return Vector4Node(this).sub(b); }
const OperandNode *ValueNode::mul(const Vector4Node *b) const { return Vector4Node(this).mul(b); }
const OperandNode *ValueNode::div(const Vector4Node *b) const { return Vector4Node(this).div(b); }
const OperandNode *ValueNode::intMod(const Vector4Node *b) const { return Vector4Node(this).intMod(b); }
const OperandNode *ValueNode::less(const Vector4Node *b) const { return Vector4Node(this).less(b); }
const OperandNode *ValueNode::lessEq(const Vector4Node *b) const { return Vector4Node(this).lessEq(b); }
const OperandNode *ValueNode::greater(const Vector4Node *b) const { return Vector4Node(this).greater(b); }
const OperandNode *ValueNode::greaterEq(const Vector4Node *b) const { return Vector4Node(this).greaterEq(b); }
const OperandNode *ValueNode::equal(const Vector4Node *b) const { return Vector4Node(this).equal(b); }
const OperandNode *ValueNode::notEqual(const Vector4Node *b) const { return Vector4Node(this).notEqual(b); }
const OperandNode *ValueNode::And(const Vector4Node *b) const { return Vector4Node(this).And(b); }
const OperandNode *ValueNode::Or(const Vector4Node *b) const { return Vector4Node(this).Or(b); }
const OperandNode *ValueNode::min(const Vector4Node *b) const { return Vector4Node(this).min(b); }
const OperandNode *ValueNode::max(const Vector4Node *b) const { return Vector4Node(this).max(b); }
const OperandNode *ValueNode::pow(const Vector4Node *b) const { return Vector4Node(this).pow(b); }
const OperandNode *ValueNode::fmod(const Vector4Node *b) const { return Vector4Node(this).fmod(b); }
const OperandNode *ValueNode::atan2(const Vector4Node *b) const { return Vector4Node(this).atan2(b); }
const OperandNode *ValueNode::ldexp(const Vector4Node *b) const { return Vector4Node(this).ldexp(b); }
const OperandNode *ValueNode::step(const Vector4Node *b) const { return Vector4Node(this).step(b); }
const OperandNode *ValueNode::dot(const Vector4Node *b) const { return Vector4Node(this).dot(b); }
const OperandNode *ValueNode::distance(const Vector4Node *b) const { return Vector4Node(this).distance(b); }
const OperandNode *ValueNode::mulFunc(const Vector4Node *b) const { return Vector4Node(this).mulFunc(b); }
const OperandNode *ValueNode::reflect(const Vector4Node *b) const { return Vector4Node(this).reflect(b); }

const OperandNode *ValueNode::add(const Vector3Node *b) const { return Vector3Node(this).add(b); }
const OperandNode *ValueNode::sub(const Vector3Node *b) const { return Vector3Node(this).sub(b); }
const OperandNode *ValueNode::mul(const Vector3Node *b) const { return Vector3Node(this).mul(b); }
const OperandNode *ValueNode::div(const Vector3Node *b) const { return Vector3Node(this).div(b); }
const OperandNode *ValueNode::intMod(const Vector3Node *b) const { return Vector3Node(this).intMod(b); }
const OperandNode *ValueNode::less(const Vector3Node *b) const { return Vector3Node(this).less(b); }
const OperandNode *ValueNode::lessEq(const Vector3Node *b) const { return Vector3Node(this).lessEq(b); }
const OperandNode *ValueNode::greater(const Vector3Node *b) const { return Vector3Node(this).greater(b); }
const OperandNode *ValueNode::greaterEq(const Vector3Node *b) const { return Vector3Node(this).greaterEq(b); }
const OperandNode *ValueNode::equal(const Vector3Node *b) const { return Vector3Node(this).equal(b); }
const OperandNode *ValueNode::notEqual(const Vector3Node *b) const { return Vector3Node(this).notEqual(b); }
const OperandNode *ValueNode::And(const Vector3Node *b) const { return Vector3Node(this).And(b); }
const OperandNode *ValueNode::Or(const Vector3Node *b) const { return Vector3Node(this).Or(b); }
const OperandNode *ValueNode::min(const Vector3Node *b) const { return Vector3Node(this).min(b); }
const OperandNode *ValueNode::max(const Vector3Node *b) const { return Vector3Node(this).max(b); }
const OperandNode *ValueNode::pow(const Vector3Node *b) const { return Vector3Node(this).pow(b); }
const OperandNode *ValueNode::fmod(const Vector3Node *b) const { return Vector3Node(this).fmod(b); }
const OperandNode *ValueNode::atan2(const Vector3Node *b) const { return Vector3Node(this).atan2(b); }
const OperandNode *ValueNode::ldexp(const Vector3Node *b) const { return Vector3Node(this).ldexp(b); }
const OperandNode *ValueNode::step(const Vector3Node *b) const { return Vector3Node(this).step(b); }
const OperandNode *ValueNode::dot(const Vector3Node *b) const { return Vector3Node(this).dot(b); }
const OperandNode *ValueNode::cross(const Vector3Node *b) const { return Vector3Node(this).cross(b); }
const OperandNode *ValueNode::distance(const Vector3Node *b) const { return Vector3Node(this).distance(b); }
const OperandNode *ValueNode::mulFunc(const Vector3Node *b) const { return Vector3Node(this).mulFunc(b); }
const OperandNode *ValueNode::reflect(const Vector3Node *b) const { return Vector3Node(this).reflect(b); }

const OperandNode *ValueNode::add(const Vector2Node *b) const { return Vector2Node(this).add(b); }
const OperandNode *ValueNode::sub(const Vector2Node *b) const { return Vector2Node(this).sub(b); }
const OperandNode *ValueNode::mul(const Vector2Node *b) const { return Vector2Node(this).mul(b); }
const OperandNode *ValueNode::div(const Vector2Node *b) const { return Vector2Node(this).div(b); }
const OperandNode *ValueNode::intMod(const Vector2Node *b) const { return Vector2Node(this).intMod(b); }
const OperandNode *ValueNode::less(const Vector2Node *b) const { return Vector2Node(this).less(b); }
const OperandNode *ValueNode::lessEq(const Vector2Node *b) const { return Vector2Node(this).lessEq(b); }
const OperandNode *ValueNode::greater(const Vector2Node *b) const { return Vector2Node(this).greater(b); }
const OperandNode *ValueNode::greaterEq(const Vector2Node *b) const { return Vector2Node(this).greaterEq(b); }
const OperandNode *ValueNode::equal(const Vector2Node *b) const { return Vector2Node(this).equal(b); }
const OperandNode *ValueNode::notEqual(const Vector2Node *b) const { return Vector2Node(this).notEqual(b); }
const OperandNode *ValueNode::And(const Vector2Node *b) const { return Vector2Node(this).And(b); }
const OperandNode *ValueNode::Or(const Vector2Node *b) const { return Vector2Node(this).Or(b); }
const OperandNode *ValueNode::min(const Vector2Node *b) const { return Vector2Node(this).min(b); }
const OperandNode *ValueNode::max(const Vector2Node *b) const { return Vector2Node(this).max(b); }
const OperandNode *ValueNode::pow(const Vector2Node *b) const { return Vector2Node(this).pow(b); }
const OperandNode *ValueNode::fmod(const Vector2Node *b) const { return Vector2Node(this).fmod(b); }
const OperandNode *ValueNode::atan2(const Vector2Node *b) const { return Vector2Node(this).atan2(b); }
const OperandNode *ValueNode::ldexp(const Vector2Node *b) const { return Vector2Node(this).ldexp(b); }
const OperandNode *ValueNode::step(const Vector2Node *b) const { return Vector2Node(this).step(b); }
const OperandNode *ValueNode::dot(const Vector2Node *b) const { return Vector2Node(this).dot(b); }
const OperandNode *ValueNode::distance(const Vector2Node *b) const { return Vector2Node(this).distance(b); }
const OperandNode *ValueNode::mulFunc(const Vector2Node *b) const { return Vector2Node(this).mulFunc(b); }
const OperandNode *ValueNode::reflect(const Vector2Node *b) const { return Vector2Node(this).reflect(b); }

const OperandNode *ValueNode::add(const Matrix44Node *b) const { return Matrix44Node(this).add(b); }
const OperandNode *ValueNode::sub(const Matrix44Node *b) const { return Matrix44Node(this).sub(b); }
const OperandNode *ValueNode::mul(const Matrix44Node *b) const { return Matrix44Node(this).mul(b); }
const OperandNode *ValueNode::div(const Matrix44Node *b) const { return Matrix44Node(this).div(b); }
const OperandNode *ValueNode::intMod(const Matrix44Node *b) const { return Matrix44Node(this).intMod(b); }
const OperandNode *ValueNode::less(const Matrix44Node *b) const { return Matrix44Node(this).less(b); }
const OperandNode *ValueNode::lessEq(const Matrix44Node *b) const { return Matrix44Node(this).lessEq(b); }
const OperandNode *ValueNode::greater(const Matrix44Node *b) const { return Matrix44Node(this).greater(b); }
const OperandNode *ValueNode::greaterEq(const Matrix44Node *b) const { return Matrix44Node(this).greaterEq(b); }
const OperandNode *ValueNode::equal(const Matrix44Node *b) const { return Matrix44Node(this).equal(b); }
const OperandNode *ValueNode::notEqual(const Matrix44Node *b) const { return Matrix44Node(this).notEqual(b); }
const OperandNode *ValueNode::And(const Matrix44Node *b) const { return Matrix44Node(this).And(b); }
const OperandNode *ValueNode::Or(const Matrix44Node *b) const { return Matrix44Node(this).Or(b); }
const OperandNode *ValueNode::min(const Matrix44Node *b) const { return Matrix44Node(this).min(b); }
const OperandNode *ValueNode::max(const Matrix44Node *b) const { return Matrix44Node(this).max(b); }
const OperandNode *ValueNode::pow(const Matrix44Node *b) const { return Matrix44Node(this).pow(b); }
const OperandNode *ValueNode::fmod(const Matrix44Node *b) const { return Matrix44Node(this).fmod(b); }
const OperandNode *ValueNode::atan2(const Matrix44Node *b) const { return Matrix44Node(this).atan2(b); }
const OperandNode *ValueNode::ldexp(const Matrix44Node *b) const { return Matrix44Node(this).ldexp(b); }
const OperandNode *ValueNode::step(const Matrix44Node *b) const { return Matrix44Node(this).step(b); }
const OperandNode *ValueNode::mulFunc(const Matrix44Node *b) const { return Matrix44Node(this).mulFunc(b); }

const OperandNode *ValueNode::clamp(const ValueNode *b, const ValueNode *c) const { return new ValueNode(std::min(std::max(v, b->v), c->v)); }
const OperandNode *ValueNode::clamp(const Vector4Node *b, const Vector4Node *c) const { return Vector4Node(this).clamp(b, c); }
const OperandNode *ValueNode::clamp(const Vector3Node *b, const Vector3Node *c) const { return Vector3Node(this).clamp(b, c); }
const OperandNode *ValueNode::clamp(const Vector2Node *b, const Vector2Node *c) const { return Vector2Node(this).clamp(b, c); }
const OperandNode *ValueNode::b_clamp(const OperandNode *a, const ValueNode *c) const { return a->clamp(this, c); }
const OperandNode* ValueNode::b_clamp(const OperandNode* a, const Vector4Node* c) const { Vector4Node t(this); return a->clamp(&t, c); }
const OperandNode *ValueNode::b_clamp(const OperandNode *a, const Vector3Node *c) const { Vector3Node t(this); return a->clamp(&t, c); }
const OperandNode *ValueNode::b_clamp(const OperandNode *a, const Vector2Node *c) const { Vector2Node t(this); return a->clamp(&t, c); }
const OperandNode *ValueNode::c_clamp(const OperandNode *a, const OperandNode *b) const { return b->b_clamp(a, this); }

const OperandNode *ValueNode::lerp(const ValueNode *b, const ValueNode *c) const { return new ValueNode(v + (b->v - v) * c->v); }
const OperandNode *ValueNode::lerp(const Vector4Node *b, const Vector4Node *c) const { return Vector4Node(this).lerp(b, c); }
const OperandNode *ValueNode::lerp(const Vector3Node *b, const Vector3Node *c) const { return Vector3Node(this).lerp(b, c); }
const OperandNode *ValueNode::lerp(const Vector2Node *b, const Vector2Node *c) const { return Vector2Node(this).lerp(b, c); }
const OperandNode *ValueNode::b_lerp(const OperandNode *a, const ValueNode *c) const { return a->lerp(this, c); }
const OperandNode* ValueNode::b_lerp(const OperandNode* a, const Vector4Node* c) const { Vector4Node t(this);  return a->lerp(&t, c); }
const OperandNode* ValueNode::b_lerp(const OperandNode* a, const Vector3Node* c) const { Vector3Node t(this);  return a->lerp(&t, c); }
const OperandNode* ValueNode::b_lerp(const OperandNode* a, const Vector2Node* c) const { Vector2Node t(this);  return a->lerp(&t, c); }
const OperandNode *ValueNode::c_lerp(const OperandNode *a, const OperandNode *b) const { return b->b_lerp(a, this); }

const OperandNode *ValueNode::mad(const ValueNode *b, const ValueNode *c) const { return new ValueNode(v * b->v + c->v); }
const OperandNode *ValueNode::mad(const Vector4Node *b, const Vector4Node *c) const { return Vector4Node(this).mad(b, c); }
const OperandNode *ValueNode::mad(const Vector3Node *b, const Vector3Node *c) const { return Vector3Node(this).mad(b, c); }
const OperandNode *ValueNode::mad(const Vector2Node *b, const Vector2Node *c) const { return Vector2Node(this).mad(b, c); }
const OperandNode *ValueNode::b_mad(const OperandNode *a, const ValueNode *c) const { return a->mad(this, c); }
const OperandNode* ValueNode::b_mad(const OperandNode* a, const Vector4Node* c) const { Vector4Node t(this); return a->mad(&t, c); }
const OperandNode* ValueNode::b_mad(const OperandNode* a, const Vector3Node* c) const { Vector3Node t(this); return a->mad(&t, c); }
const OperandNode* ValueNode::b_mad(const OperandNode* a, const Vector2Node* c) const { Vector2Node t(this); return a->mad(&t, c); }
const OperandNode *ValueNode::c_mad(const OperandNode *a, const OperandNode *b) const { return b->b_mad(a, this); }

const OperandNode *ValueNode::smoothstep(const ValueNode *b, const ValueNode *c) const { float64 t = (c->v - v) / (b->v - v); t = std::min(std::max(t, 0.0), 1.0); return new ValueNode(t * t * (3.0 - 2.0 * t)); }
const OperandNode *ValueNode::smoothstep(const Vector4Node *b, const Vector4Node *c) const { return Vector4Node(this).smoothstep(b, c); }
const OperandNode *ValueNode::smoothstep(const Vector3Node *b, const Vector3Node *c) const { return Vector3Node(this).smoothstep(b, c); }
const OperandNode *ValueNode::smoothstep(const Vector2Node *b, const Vector2Node *c) const { return Vector2Node(this).smoothstep(b, c); }
const OperandNode *ValueNode::b_smoothstep(const OperandNode *a, const ValueNode *c) const { return a->smoothstep(this, c); }
const OperandNode *ValueNode::b_smoothstep(const OperandNode *a, const Vector4Node *c) const { Vector4Node t(this); return a->smoothstep(&t, c); }
const OperandNode* ValueNode::b_smoothstep(const OperandNode* a, const Vector3Node* c) const { Vector3Node t(this); return a->smoothstep(&t, c); }
const OperandNode* ValueNode::b_smoothstep(const OperandNode* a, const Vector2Node* c) const { Vector2Node t(this); return a->smoothstep(&t, c); }
const OperandNode *ValueNode::c_smoothstep(const OperandNode *a, const OperandNode *b) const { return b->b_smoothstep(a, this); }

const OperandNode *ValueNode::ifThenElse(const ValueNode *b, const ValueNode *c) const { return new ValueNode(v != 0.0 ? b->v : c->v); }
const OperandNode *ValueNode::ifThenElse(const Vector4Node *b, const Vector4Node *c) const { return new Vector4Node(v != 0.0 ? b->v : c->v); }
const OperandNode *ValueNode::ifThenElse(const Vector3Node *b, const Vector3Node *c) const { return new Vector3Node(v != 0.0 ? b->v : c->v); }
const OperandNode *ValueNode::ifThenElse(const Vector2Node *b, const Vector2Node *c) const { return new Vector2Node(v != 0.0 ? b->v : c->v); }
const OperandNode *ValueNode::b_ifThenElse(const OperandNode *a, const ValueNode *c) const { return a->ifThenElse(this, c); }
const OperandNode* ValueNode::b_ifThenElse(const OperandNode* a, const Vector4Node* c) const { Vector4Node t(this); return a->ifThenElse(&t, c); }
const OperandNode* ValueNode::b_ifThenElse(const OperandNode* a, const Vector3Node* c) const { Vector3Node t(this); return a->ifThenElse(&t, c); }
const OperandNode* ValueNode::b_ifThenElse(const OperandNode* a, const Vector2Node* c) const { Vector2Node t(this); return a->ifThenElse(&t, c); }
const OperandNode *ValueNode::c_ifThenElse(const OperandNode *a, const OperandNode *b) const { return b->b_ifThenElse(a, this); }

const OperandNode *VectorNode::neg() const { return duplicate(XMVectorNegate(v)); }
const OperandNode *VectorNode::pos() const { return duplicate(v); }
const OperandNode *VectorNode::Not() const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v, XMVectorZero()))); }
const OperandNode *VectorNode::saturate() const { return duplicate(XMVectorSaturate(v)); }
const OperandNode *VectorNode::sin() const { return duplicate(XMVectorSin(v)); }
const OperandNode *VectorNode::cos() const { return duplicate(XMVectorCos(v)); }
const OperandNode *VectorNode::tan() const { return duplicate(XMVectorTan(v)); }
const OperandNode *VectorNode::asin() const { return duplicate(XMVectorASin(v)); }
const OperandNode *VectorNode::acos() const { return duplicate(XMVectorACos(v)); }
const OperandNode *VectorNode::atan() const { return duplicate(XMVectorATan(v)); }
const OperandNode *VectorNode::abs() const { return duplicate(XMVectorAbs(v)); }
const OperandNode *VectorNode::sqrt() const { return duplicate(XMVectorSqrt(v)); }
const OperandNode *VectorNode::rsqrt() const { return duplicate(XMVectorDivide(XMVectorSplatOne(), XMVectorSqrt(v))); }
const OperandNode *VectorNode::floor() const { return duplicate(XMVectorFloor(v)); }
const OperandNode *VectorNode::ceil() const { return duplicate(XMVectorCeiling(v)); }
const OperandNode *VectorNode::round() const { return duplicate(XMVectorRound(v)); }
const OperandNode *VectorNode::sinh() const { return duplicate(XMVectorSinH(v)); }
const OperandNode *VectorNode::cosh() const { return duplicate(XMVectorCosH(v)); }
const OperandNode *VectorNode::tanh() const { return duplicate(XMVectorTanH(v)); }
const OperandNode *VectorNode::log10() const { return duplicate(XMVectorLog(v)); }
const OperandNode *VectorNode::log() const { return duplicate(XMVectorLogE(v)); }
const OperandNode *VectorNode::log2() const { return duplicate(XMVectorLog2(v)); }
const OperandNode *VectorNode::exp() const { return duplicate(XMVectorExpE(v)); }
const OperandNode *VectorNode::exp2() const { return duplicate(XMVectorExp2(v)); }
const OperandNode *VectorNode::frac() const { return duplicate(XMVectorSubtract(v, XMVectorFloor(v))); }
const OperandNode *VectorNode::sign() const { return duplicate(XMVectorSelect(XMVectorSelect(XMVectorNegate(XMVectorSplatOne()), XMVectorSplatOne(), XMVectorGreater(v, XMVectorZero())), XMVectorZero(), XMVectorEqual(v, XMVectorZero()))); }
const OperandNode *VectorNode::rad() const { return duplicate(XMVectorScale(v, TORAD)); }
const OperandNode *VectorNode::deg() const { return duplicate(XMVectorScale(v, TODEG)); }
const OperandNode *VectorNode::isNaN() const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorIsNaN(v))); }
const OperandNode *VectorNode::isInf() const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorIsInfinite(v))); }
const OperandNode *VectorNode::trunc() const { return duplicate(XMVectorTruncate(v)); }
const OperandNode *VectorNode::swizzle(uint32 mask) const
{
	uint32 x = mask & 0x03, y = (mask >> 8) & 0x03, z = (mask >> 16) & 0x03, w = (mask >> 24) & 0x03;
	uint32 n = ncomp();
	if (x > n || y > n || z > n || w > n)
		throw InvalidDimensionException();
	XMVECTOR r = XMVectorSwizzle(v, x, y, z, w);
	if ((mask & 0xFFFFFF00) == 0xFFFFFF00)
		return new ValueNode(XMVectorGetX(r));
	else if ((mask & 0xFFFF0000) == 0xFFFF0000)
		return new Vector2Node(r);
	else if ((mask & 0xFF000000) == 0xFF000000)
		return new Vector3Node(r);
	return new Vector4Node(r);
}

const OperandNode *VectorNode::subscript(const ValueNode *b) const
{
	uint32 idx = (uint32)b->v;
	if (idx >= ncomp())
		throw ExprNotImplException(); // something else to throw!!!
	return new ValueNode(XMVectorGetByIndex(v, idx));
}

const OperandNode *VectorNode::add(const VectorNode *b) const { return duplicate(XMVectorAdd(v, b->v)); }
const OperandNode *VectorNode::sub(const VectorNode *b) const { return duplicate(XMVectorSubtract(v, b->v)); }
const OperandNode *VectorNode::mul(const VectorNode *b) const { return duplicate(XMVectorMultiply(v, b->v)); }
const OperandNode *VectorNode::div(const VectorNode *b) const { return duplicate(XMVectorDivide(v, b->v)); }
const OperandNode *VectorNode::intMod(const VectorNode *b) const { return fmod(b); }
const OperandNode *VectorNode::less(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLess(v, b->v))); }
const OperandNode *VectorNode::lessEq(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLessOrEqual(v, b->v))); }
const OperandNode *VectorNode::greater(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreater(v, b->v))); }
const OperandNode *VectorNode::greaterEq(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreaterOrEqual(v, b->v))); }
const OperandNode *VectorNode::equal(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v, b->v))); }
const OperandNode *VectorNode::notEqual(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorNotEqual(v, b->v))); }
const OperandNode *VectorNode::And(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorOrInt(XMVectorEqual(v, XMVectorZero()), XMVectorEqual(b->v, XMVectorZero())))); }
const OperandNode *VectorNode::Or(const VectorNode *b) const { return duplicate(XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorAndInt(XMVectorEqual(v, XMVectorZero()), XMVectorEqual(b->v, XMVectorZero())))); }
const OperandNode *VectorNode::min(const VectorNode *b) const { return duplicate(XMVectorMin(v, b->v)); }
const OperandNode *VectorNode::max(const VectorNode *b) const { return duplicate(XMVectorMax(v, b->v)); }
const OperandNode *VectorNode::pow(const VectorNode *b) const { return duplicate(XMVectorPow(v, b->v)); }
const OperandNode *VectorNode::fmod(const VectorNode *b) const { return duplicate(XMVectorMod(v, b->v)); }
const OperandNode *VectorNode::atan2(const VectorNode *b) const { return duplicate(XMVectorATan2(v, b->v)); }
const OperandNode *VectorNode::ldexp(const VectorNode *b) const { return duplicate(XMVectorMultiply(v, XMVectorExp2(b->v))); }
const OperandNode *VectorNode::step(const VectorNode *b) const { return greaterEq(b); }

const OperandNode *Vector4Node::all() const { return new ValueNode(XMComparisonAllFalse(XMVector4EqualR(v, XMVectorZero())) ? 1.0f : 0.0f); }
const OperandNode *Vector4Node::any() const { return new ValueNode(XMComparisonAnyFalse(XMVector4EqualR(v, XMVectorZero())) ? 1.0f : 0.0f); }
const OperandNode *Vector4Node::length() const { return new ValueNode(XMVectorGetX(XMVector4Length(v))); }
const OperandNode *Vector4Node::normalize() const { return duplicate(XMVector4Normalize(v)); }

const OperandNode *Vector4Node::add(const ValueNode *b) const { Vector4Node t(b); return add(&t); }
const OperandNode *Vector4Node::sub(const ValueNode *b) const { Vector4Node t(b); return sub(&t); }
const OperandNode *Vector4Node::mul(const ValueNode *b) const { Vector4Node t(b); return mul(&t); }
const OperandNode *Vector4Node::div(const ValueNode *b) const { Vector4Node t(b); return div(&t); }
const OperandNode *Vector4Node::intMod(const ValueNode *b) const { Vector4Node t(b); return intMod(&t); }
const OperandNode *Vector4Node::less(const ValueNode *b) const { Vector4Node t(b); return less(&t); }
const OperandNode *Vector4Node::lessEq(const ValueNode *b) const { Vector4Node t(b); return lessEq(&t); }
const OperandNode *Vector4Node::greater(const ValueNode *b) const { Vector4Node t(b); return greater(&t); }
const OperandNode *Vector4Node::greaterEq(const ValueNode *b) const { Vector4Node t(b); return greaterEq(&t); }
const OperandNode *Vector4Node::equal(const ValueNode *b) const { Vector4Node t(b); return equal(&t); }
const OperandNode *Vector4Node::notEqual(const ValueNode *b) const { Vector4Node t(b); return notEqual(&t); }
const OperandNode *Vector4Node::And(const ValueNode *b) const { Vector4Node t(b); return And(&t); }
const OperandNode *Vector4Node::Or(const ValueNode *b) const { Vector4Node t(b); return Or(&t); }
const OperandNode *Vector4Node::min(const ValueNode *b) const { Vector4Node t(b); return min(&t); }
const OperandNode *Vector4Node::max(const ValueNode *b) const { Vector4Node t(b); return max(&t); }
const OperandNode *Vector4Node::pow(const ValueNode *b) const { Vector4Node t(b); return pow(&t); }
const OperandNode *Vector4Node::fmod(const ValueNode *b) const { Vector4Node t(b); return fmod(&t); }
const OperandNode *Vector4Node::atan2(const ValueNode *b) const { Vector4Node t(b); return atan2(&t); }
const OperandNode *Vector4Node::ldexp(const ValueNode *b) const { Vector4Node t(b); return ldexp(&t); }
const OperandNode *Vector4Node::step(const ValueNode *b) const { Vector4Node t(b); return step(&t); }
const OperandNode *Vector4Node::dot(const ValueNode *b) const { Vector4Node t(b); return dot(&t); }
const OperandNode *Vector4Node::distance(const ValueNode *b) const { Vector4Node t(b); return distance(&t); }
const OperandNode *Vector4Node::mulFunc(const ValueNode *b) const { Vector4Node t(b); return mulFunc(&t); }
const OperandNode *Vector4Node::reflect(const ValueNode *b) const { Vector4Node t(b); return reflect(&t); }

const OperandNode *Vector4Node::add(const Vector4Node *b) const { return VectorNode::add(b); }
const OperandNode *Vector4Node::sub(const Vector4Node *b) const { return VectorNode::sub(b); }
const OperandNode *Vector4Node::mul(const Vector4Node *b) const { return VectorNode::mul(b); }
const OperandNode *Vector4Node::div(const Vector4Node *b) const { return VectorNode::div(b); }
const OperandNode *Vector4Node::intMod(const Vector4Node *b) const { return VectorNode::intMod(b); }
const OperandNode *Vector4Node::less(const Vector4Node *b) const { return VectorNode::less(b); }
const OperandNode *Vector4Node::lessEq(const Vector4Node *b) const { return VectorNode::lessEq(b); }
const OperandNode *Vector4Node::greater(const Vector4Node *b) const { return VectorNode::greater(b); }
const OperandNode *Vector4Node::greaterEq(const Vector4Node *b) const { return VectorNode::greaterEq(b); }
const OperandNode *Vector4Node::equal(const Vector4Node *b) const { return VectorNode::equal(b); }
const OperandNode *Vector4Node::notEqual(const Vector4Node *b) const { return VectorNode::notEqual(b); }
const OperandNode *Vector4Node::And(const Vector4Node *b) const { return VectorNode::And(b); }
const OperandNode *Vector4Node::Or(const Vector4Node *b) const { return VectorNode::Or(b); }
const OperandNode *Vector4Node::min(const Vector4Node *b) const { return VectorNode::min(b); }
const OperandNode *Vector4Node::max(const Vector4Node *b) const { return VectorNode::max(b); }
const OperandNode *Vector4Node::pow(const Vector4Node *b) const { return VectorNode::pow(b); }
const OperandNode *Vector4Node::fmod(const Vector4Node *b) const { return VectorNode::fmod(b); }
const OperandNode *Vector4Node::atan2(const Vector4Node *b) const { return VectorNode::atan2(b); }
const OperandNode *Vector4Node::ldexp(const Vector4Node *b) const { return VectorNode::ldexp(b); }
const OperandNode *Vector4Node::step(const Vector4Node *b) const { return VectorNode::step(b); }
const OperandNode *Vector4Node::dot(const Vector4Node *b) const { return new ValueNode(XMVectorGetX(XMVector4Dot(v, b->v))); }
const OperandNode *Vector4Node::distance(const Vector4Node *b) const { return new ValueNode(XMVectorGetX(XMVector4Length(v - b->v))); }
const OperandNode *Vector4Node::mulFunc(const Vector4Node *b) const { return dot(b); }
const OperandNode *Vector4Node::reflect(const Vector4Node *b) const { return duplicate(XMVector4Reflect(v, b->v)); }

const OperandNode *Vector4Node::mul(const Matrix44Node *b) const { return mulFunc(b); }
const OperandNode *Vector4Node::mulFunc(const Matrix44Node *b) const { return duplicate(XMVector4Transform(v, XMMatrixTranspose(b->v))); }

const OperandNode *Vector3Node::mul(const Matrix33Node *b) const { return mulFunc(b); }
const OperandNode *Vector3Node::mulFunc(const Matrix33Node *b) const { return nullptr; }//duplicate(XMVector3Transform(v, XMMatrixTranspose(b->v))); }

const OperandNode *MatrixNode::neg() const { return duplicate(XMMATRIX(XMVectorNegate(v.r[0]), XMVectorNegate(v.r[1]), XMVectorNegate(v.r[2]), XMVectorNegate(v.r[3]))); }
const OperandNode *MatrixNode::pos() const { return duplicate(v); }
const OperandNode *MatrixNode::Not() const { return duplicate(XMMATRIX(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[0], XMVectorZero())), XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[1], XMVectorZero())), XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[2], XMVectorZero())), XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[3], XMVectorZero())))); }
const OperandNode *MatrixNode::saturate() const { return duplicate(XMMATRIX(XMVectorSaturate(v.r[0]), XMVectorSaturate(v.r[1]), XMVectorSaturate(v.r[2]), XMVectorSaturate(v.r[3]))); }
const OperandNode *MatrixNode::sin() const { return duplicate(XMMATRIX(XMVectorSin(v.r[0]), XMVectorSin(v.r[1]), XMVectorSin(v.r[2]), XMVectorSin(v.r[3]))); }
const OperandNode *MatrixNode::cos() const { return duplicate(XMMATRIX(XMVectorCos(v.r[0]), XMVectorCos(v.r[1]), XMVectorCos(v.r[2]), XMVectorCos(v.r[3]))); }
const OperandNode *MatrixNode::tan() const { return duplicate(XMMATRIX(XMVectorTan(v.r[0]), XMVectorTan(v.r[1]), XMVectorTan(v.r[2]), XMVectorTan(v.r[3]))); }
const OperandNode *MatrixNode::asin() const { return duplicate(XMMATRIX(XMVectorASin(v.r[0]), XMVectorASin(v.r[1]), XMVectorASin(v.r[2]), XMVectorASin(v.r[3]))); }
const OperandNode *MatrixNode::acos() const { return duplicate(XMMATRIX(XMVectorACos(v.r[0]), XMVectorACos(v.r[1]), XMVectorACos(v.r[2]), XMVectorACos(v.r[3]))); }
const OperandNode *MatrixNode::atan() const { return duplicate(XMMATRIX(XMVectorATan(v.r[0]), XMVectorATan(v.r[1]), XMVectorATan(v.r[2]), XMVectorATan(v.r[3]))); }
const OperandNode *MatrixNode::abs() const { return duplicate(XMMATRIX(XMVectorAbs(v.r[0]), XMVectorAbs(v.r[1]), XMVectorAbs(v.r[2]), XMVectorAbs(v.r[3]))); }
const OperandNode *MatrixNode::sqrt() const { return duplicate(XMMATRIX(XMVectorSqrt(v.r[0]), XMVectorSqrt(v.r[1]), XMVectorSqrt(v.r[2]), XMVectorSqrt(v.r[3]))); }
const OperandNode *MatrixNode::rsqrt() const { return duplicate(XMMATRIX(XMVectorDivide(XMVectorSplatOne(), XMVectorSqrt(v.r[0])), XMVectorDivide(XMVectorSplatOne(), XMVectorSqrt(v.r[1])), XMVectorDivide(XMVectorSplatOne(), XMVectorSqrt(v.r[2])), XMVectorDivide(XMVectorSplatOne(), XMVectorSqrt(v.r[3])))); }
const OperandNode *MatrixNode::floor() const { return duplicate(XMMATRIX(XMVectorFloor(v.r[0]), XMVectorFloor(v.r[1]), XMVectorFloor(v.r[2]), XMVectorFloor(v.r[3]))); }
const OperandNode *MatrixNode::ceil() const { return duplicate(XMMATRIX(XMVectorCeiling(v.r[0]), XMVectorCeiling(v.r[1]), XMVectorCeiling(v.r[2]), XMVectorCeiling(v.r[3]))); }
const OperandNode *MatrixNode::round() const { return duplicate(XMMATRIX(XMVectorRound(v.r[0]), XMVectorRound(v.r[1]), XMVectorRound(v.r[2]), XMVectorRound(v.r[3]))); }
const OperandNode *MatrixNode::sinh() const { return duplicate(XMMATRIX(XMVectorSinH(v.r[0]), XMVectorSinH(v.r[1]), XMVectorSinH(v.r[2]), XMVectorSinH(v.r[3]))); }
const OperandNode *MatrixNode::cosh() const { return duplicate(XMMATRIX(XMVectorCosH(v.r[0]), XMVectorCosH(v.r[1]), XMVectorCosH(v.r[2]), XMVectorCosH(v.r[3]))); }
const OperandNode *MatrixNode::tanh() const { return duplicate(XMMATRIX(XMVectorTanH(v.r[0]), XMVectorTanH(v.r[1]), XMVectorTanH(v.r[2]), XMVectorTanH(v.r[3]))); }
const OperandNode *MatrixNode::log10() const { return duplicate(XMMATRIX(XMVectorLog(v.r[0]), XMVectorLog(v.r[1]), XMVectorLog(v.r[2]), XMVectorLog(v.r[3]))); }
const OperandNode *MatrixNode::log() const { return duplicate(XMMATRIX(XMVectorLogE(v.r[0]), XMVectorLogE(v.r[1]), XMVectorLogE(v.r[2]), XMVectorLogE(v.r[3]))); }
const OperandNode *MatrixNode::log2() const { return duplicate(XMMATRIX(XMVectorLog2(v.r[0]), XMVectorLog2(v.r[1]), XMVectorLog2(v.r[2]), XMVectorLog2(v.r[3]))); }
const OperandNode *MatrixNode::exp() const { return duplicate(XMMATRIX(XMVectorExp(v.r[0]), XMVectorExp(v.r[1]), XMVectorExp(v.r[2]), XMVectorExp(v.r[3]))); }
const OperandNode *MatrixNode::exp2() const { return duplicate(XMMATRIX(XMVectorExp2(v.r[0]), XMVectorExp2(v.r[1]), XMVectorExp2(v.r[2]), XMVectorExp2(v.r[3]))); }
const OperandNode *MatrixNode::frac() const { return duplicate(XMMATRIX(XMVectorSubtract(v.r[0], XMVectorFloor(v.r[0])), XMVectorSubtract(v.r[1], XMVectorFloor(v.r[1])), XMVectorSubtract(v.r[2], XMVectorFloor(v.r[2])), XMVectorSubtract(v.r[3], XMVectorFloor(v.r[3])))); }
const OperandNode *MatrixNode::sign() const {
	return duplicate(XMMATRIX(
		XMVectorSelect(XMVectorSelect(XMVectorNegate(XMVectorSplatOne()), XMVectorSplatOne(), XMVectorGreater(v.r[0], XMVectorZero())), XMVectorZero(), XMVectorEqual(v.r[0], XMVectorZero())),
		XMVectorSelect(XMVectorSelect(XMVectorNegate(XMVectorSplatOne()), XMVectorSplatOne(), XMVectorGreater(v.r[1], XMVectorZero())), XMVectorZero(), XMVectorEqual(v.r[1], XMVectorZero())),
		XMVectorSelect(XMVectorSelect(XMVectorNegate(XMVectorSplatOne()), XMVectorSplatOne(), XMVectorGreater(v.r[2], XMVectorZero())), XMVectorZero(), XMVectorEqual(v.r[2], XMVectorZero())),
		XMVectorSelect(XMVectorSelect(XMVectorNegate(XMVectorSplatOne()), XMVectorSplatOne(), XMVectorGreater(v.r[3], XMVectorZero())), XMVectorZero(), XMVectorEqual(v.r[3], XMVectorZero()))));
}
const OperandNode *MatrixNode::rad() const { return duplicate(XMMATRIX(XMVectorScale(v.r[0], TORAD), XMVectorScale(v.r[1], TORAD), XMVectorScale(v.r[2], TORAD), XMVectorScale(v.r[3], TORAD))); }
const OperandNode *MatrixNode::deg() const { return duplicate(XMMATRIX(XMVectorScale(v.r[0], TODEG), XMVectorScale(v.r[1], TODEG), XMVectorScale(v.r[2], TODEG), XMVectorScale(v.r[3], TODEG))); }
const OperandNode *MatrixNode::isNaN() const { return new ValueNode(XMMatrixIsNaN(v) ? 1.0 : 0.0); }
const OperandNode *MatrixNode::isInf() const { return new ValueNode(XMMatrixIsInfinite(v) ? 1.0 : 0.0); }
const OperandNode *MatrixNode::trunc() const { return duplicate(XMMATRIX(XMVectorTruncate(v.r[0]), XMVectorTruncate(v.r[1]), XMVectorTruncate(v.r[2]), XMVectorTruncate(v.r[3]))); }

const OperandNode *MatrixNode::add(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorAdd(v.r[0], b->v.r[0]), XMVectorAdd(v.r[1], b->v.r[1]), XMVectorAdd(v.r[2], b->v.r[2]), XMVectorAdd(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::sub(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorSubtract(v.r[0], b->v.r[0]), XMVectorSubtract(v.r[1], b->v.r[1]), XMVectorSubtract(v.r[2], b->v.r[2]), XMVectorSubtract(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::mul(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorMultiply(v.r[0], b->v.r[0]), XMVectorMultiply(v.r[1], b->v.r[1]), XMVectorMultiply(v.r[2], b->v.r[2]), XMVectorMultiply(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::div(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorDivide(v.r[0], b->v.r[0]), XMVectorDivide(v.r[1], b->v.r[1]), XMVectorDivide(v.r[2], b->v.r[2]), XMVectorDivide(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::intMod(const MatrixNode *b) const { return fmod(b); }
const OperandNode *MatrixNode::less(const MatrixNode *b) const { return duplicate(XMMATRIX(
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLess(v.r[0], b->v.r[0])), 
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLess(v.r[1], b->v.r[1])), 
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLess(v.r[2], b->v.r[2])), 
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLess(v.r[3], b->v.r[3])))); }
const OperandNode *MatrixNode::lessEq(const MatrixNode *b) const { return duplicate(XMMATRIX(
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLessOrEqual(v.r[0], b->v.r[0])), 
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLessOrEqual(v.r[1], b->v.r[1])), 
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLessOrEqual(v.r[2], b->v.r[2])), 
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorLessOrEqual(v.r[3], b->v.r[3])))); }
const OperandNode *MatrixNode::greater(const MatrixNode *b) const { return duplicate(XMMATRIX(
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreater(v.r[0], b->v.r[0])), 
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreater(v.r[1], b->v.r[1])),
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreater(v.r[2], b->v.r[2])),
	XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreater(v.r[3], b->v.r[3])))); }
const OperandNode *MatrixNode::greaterEq(const MatrixNode *b) const { return duplicate(XMMATRIX(
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreaterOrEqual(v.r[0], b->v.r[0])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreaterOrEqual(v.r[1], b->v.r[1])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreaterOrEqual(v.r[2], b->v.r[2])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorGreaterOrEqual(v.r[3], b->v.r[3])))); }
const OperandNode *MatrixNode::equal(const MatrixNode *b) const { return duplicate(XMMATRIX(
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[0], b->v.r[0])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[1], b->v.r[1])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[2], b->v.r[2])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorEqual(v.r[3], b->v.r[3])))); }
const OperandNode *MatrixNode::notEqual(const MatrixNode *b) const { return duplicate(XMMATRIX(
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorNotEqual(v.r[0], b->v.r[0])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorNotEqual(v.r[1], b->v.r[1])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorNotEqual(v.r[2], b->v.r[2])),
		XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), XMVectorNotEqual(v.r[3], b->v.r[3])))); }
const OperandNode *MatrixNode::And(const MatrixNode *b) const { return duplicate(XMMATRIX(
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorOrInt(XMVectorEqual(v.r[0], XMVectorZero()), XMVectorEqual(b->v.r[0], XMVectorZero()))),
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorOrInt(XMVectorEqual(v.r[1], XMVectorZero()), XMVectorEqual(b->v.r[1], XMVectorZero()))), 
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorOrInt(XMVectorEqual(v.r[2], XMVectorZero()), XMVectorEqual(b->v.r[2], XMVectorZero()))), 
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorOrInt(XMVectorEqual(v.r[3], XMVectorZero()), XMVectorEqual(b->v.r[3], XMVectorZero()))))); }
const OperandNode *MatrixNode::Or(const MatrixNode *b) const { return duplicate(XMMATRIX(
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorAndInt(XMVectorEqual(v.r[0], XMVectorZero()), XMVectorEqual(b->v.r[0], XMVectorZero()))),
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorAndInt(XMVectorEqual(v.r[1], XMVectorZero()), XMVectorEqual(b->v.r[1], XMVectorZero()))),
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorAndInt(XMVectorEqual(v.r[2], XMVectorZero()), XMVectorEqual(b->v.r[2], XMVectorZero()))),
	XMVectorSelect(XMVectorSplatOne(), XMVectorZero(), XMVectorAndInt(XMVectorEqual(v.r[3], XMVectorZero()), XMVectorEqual(b->v.r[3], XMVectorZero()))))); }
const OperandNode *MatrixNode::min(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorMin(v.r[0], b->v.r[0]), XMVectorMin(v.r[1], b->v.r[1]), XMVectorMin(v.r[2], b->v.r[2]), XMVectorMin(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::max(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorMax(v.r[0], b->v.r[0]), XMVectorMax(v.r[1], b->v.r[1]), XMVectorMax(v.r[2], b->v.r[2]), XMVectorMax(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::pow(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorPow(v.r[0], b->v.r[0]), XMVectorPow(v.r[1], b->v.r[1]), XMVectorPow(v.r[2], b->v.r[2]), XMVectorPow(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::fmod(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorMod(v.r[0], b->v.r[0]), XMVectorMod(v.r[1], b->v.r[1]), XMVectorMod(v.r[2], b->v.r[2]), XMVectorMod(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::atan2(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorATan2(v.r[0], b->v.r[0]), XMVectorATan2(v.r[1], b->v.r[1]), XMVectorATan2(v.r[2], b->v.r[2]), XMVectorATan2(v.r[3], b->v.r[3]))); }
const OperandNode *MatrixNode::ldexp(const MatrixNode *b) const { return duplicate(XMMATRIX(XMVectorMultiply(v.r[0], XMVectorExp2(b->v.r[0])), XMVectorMultiply(v.r[1], XMVectorExp2(b->v.r[1])), XMVectorMultiply(v.r[2], XMVectorExp2(b->v.r[2])), XMVectorMultiply(v.r[3], XMVectorExp2(b->v.r[3])))); }
const OperandNode *MatrixNode::step(const MatrixNode *b) const { return greaterEq(b); }


const OperandNode *Matrix44Node::all() const { return new ValueNode(XMComparisonAllFalse(XMVector4EqualR(v.r[0], XMVectorZero())) && XMComparisonAllFalse(XMVector4EqualR(v.r[1], XMVectorZero())) && XMComparisonAllFalse(XMVector4EqualR(v.r[2], XMVectorZero())) && XMComparisonAllFalse(XMVector4EqualR(v.r[3], XMVectorZero())) ? 1.0f : 0.0f); }
const OperandNode *Matrix44Node::any() const { return new ValueNode(XMComparisonAnyFalse(XMVector4EqualR(v.r[0], XMVectorZero())) || XMComparisonAnyFalse(XMVector4EqualR(v.r[1], XMVectorZero())) || XMComparisonAnyFalse(XMVector4EqualR(v.r[2], XMVectorZero())) || XMComparisonAnyFalse(XMVector4EqualR(v.r[3], XMVectorZero())) ? 1.0f : 0.0f); }
const OperandNode *Matrix44Node::determinant() const { return new ValueNode(XMVectorGetX(XMMatrixDeterminant(v))); }
const OperandNode *Matrix44Node::inverse() const { return duplicate(XMMatrixInverse(nullptr, v)); }
const OperandNode *Matrix44Node::transpose() const { return duplicate(XMMatrixTranspose(v)); }

const OperandNode *Matrix44Node::add(const ValueNode *b) const { Matrix44Node t(b); return add(&t); }
const OperandNode *Matrix44Node::sub(const ValueNode *b) const { Matrix44Node t(b); return sub(&t); }
const OperandNode *Matrix44Node::mul(const ValueNode *b) const { Matrix44Node t(b); return mul(&t); }
const OperandNode *Matrix44Node::div(const ValueNode *b) const { Matrix44Node t(b); return div(&t); }
const OperandNode *Matrix44Node::intMod(const ValueNode *b) const { Matrix44Node t(b); return intMod(&t); }
const OperandNode *Matrix44Node::less(const ValueNode *b) const { Matrix44Node t(b); return less(&t); }
const OperandNode *Matrix44Node::lessEq(const ValueNode *b) const { Matrix44Node t(b); return lessEq(&t); }
const OperandNode *Matrix44Node::greater(const ValueNode *b) const { Matrix44Node t(b); return greater(&t); }
const OperandNode *Matrix44Node::greaterEq(const ValueNode *b) const { Matrix44Node t(b); return greaterEq(&t); }
const OperandNode *Matrix44Node::equal(const ValueNode *b) const { Matrix44Node t(b); return equal(&t); }
const OperandNode *Matrix44Node::notEqual(const ValueNode *b) const { Matrix44Node t(b); return notEqual(&t); }
const OperandNode *Matrix44Node::And(const ValueNode *b) const { Matrix44Node t(b); return And(&t); }
const OperandNode *Matrix44Node::Or(const ValueNode *b) const { Matrix44Node t(b); return Or(&t); }
const OperandNode *Matrix44Node::min(const ValueNode *b) const { Matrix44Node t(b); return min(&t); }
const OperandNode *Matrix44Node::max(const ValueNode *b) const { Matrix44Node t(b); return max(&t); }
const OperandNode *Matrix44Node::pow(const ValueNode *b) const { Matrix44Node t(b); return pow(&t); }
const OperandNode *Matrix44Node::fmod(const ValueNode *b) const { Matrix44Node t(b); return fmod(&t); }
const OperandNode *Matrix44Node::atan2(const ValueNode *b) const { Matrix44Node t(b); return atan2(&t); }
const OperandNode *Matrix44Node::ldexp(const ValueNode *b) const { Matrix44Node t(b); return ldexp(&t); }
const OperandNode *Matrix44Node::step(const ValueNode *b) const { Matrix44Node t(b); return step(&t); }
const OperandNode *Matrix44Node::mulFunc(const ValueNode *b) const { Matrix44Node t(b); return mulFunc(&t); }
const OperandNode *Matrix44Node::subscript(const ValueNode *b) const 
{
	uint32 idx = (uint32)b->v;
	if (idx >= 4)
		throw ExprNotImplException(); // something else to throw!!!
	return new Vector4Node(v.r[idx]);
}

const OperandNode *Matrix44Node::mulFunc(const Matrix44Node *b) const { return duplicate(XMMatrixMultiply(v, b->v)); }
const OperandNode *Matrix44Node::mul(const Vector4Node *b) const { return mulFunc(b); }
const OperandNode *Matrix44Node::mulFunc(const Vector4Node *b) const { return new Vector4Node(XMVector4Transform(b->v, v)); }



const OperandNode *VectorConstructorOperatorNode::operator()() const 
{
	XMVECTOR v = XMVectorZero();
	uint32 j = 0;
	for (size_t i = 0; i < n.size(); i++) {
		if (j == ccount)
			throw InvalidDimensionException();
		const OperandNode *on = (*n[n.size() - i - 1])();
		if (on->asValue())
			v = XMVectorSetByIndex(v, (float32)on->asValue()->value(), j++);
		else if (on->asVector()) {
			uint32 num = on->asVector()->ncomp();
			if (j + num > ccount)
				throw InvalidDimensionException();
			v = XMVectorSelect(v, XMVectorRotateRight(on->asVector()->v, j), XMVectorSelectControl(j < 1 ? 1 : 0, j < 2 ? 1 : 0, j < 3 ? 1 : 0, 1));
			j += num;
		}
		else
			throw InvalidDimensionException();
	}
	if (j < ccount) {
		if (j == 1)
			v = XMVectorSplatX(v);
		else
			throw InvalidDimensionException();
	}
	OperandNode *r = nullptr;
	switch (ccount)
	{
	case 2: r = new Vector2Node(v); break;
	case 3: r = new Vector3Node(v); break;
	case 4: r = new Vector4Node(v); break;
	default: throw InvalidDimensionException();
	}
	return result(r);
}
