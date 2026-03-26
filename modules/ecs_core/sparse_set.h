/**************************************************************************/
/*  sparse_set.h                                                          */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/typedefs.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include "core/variant/callable.h"

// Defines a 64-bit null check instead of 32-bit limits.
const uint64_t NULL_ENTITY = 0xFFFFFFFFFFFFFFFF;

class ISparseSet {
public:
	virtual void remove(uint64_t p_entity) = 0;
	virtual bool has(uint64_t p_entity) const = 0;
	virtual int size() const = 0;
	virtual const Vector<uint64_t> &get_dense_raw() const = 0;
	virtual ~ISparseSet() {}
};

template <typename T>
class SparseSet : public ISparseSet {
private:
	Vector<uint32_t> sparse; 
	Vector<uint64_t> dense;  
	Vector<T> components;

	// Component Lifecycle Observer Nodes implicitly notifying Servers
	Vector<Callable> on_added_observers;
	Vector<Callable> on_removed_observers;

	inline uint32_t get_index(uint64_t p_entity) const { return (uint32_t)(p_entity & 0xFFFFFFFF); }

public:
	void register_on_added(const Callable &p_callable) { on_added_observers.push_back(p_callable); }
	void register_on_removed(const Callable &p_callable) { on_removed_observers.push_back(p_callable); }
	void unregister_on_added(const Callable &p_callable) { on_added_observers.erase(p_callable); }
	void unregister_on_removed(const Callable &p_callable) { on_removed_observers.erase(p_callable); }

	void insert(uint64_t p_entity, const T &p_component) {
		uint32_t index = get_index(p_entity);
		if (index >= (uint32_t)sparse.size()) {
			int old_size = sparse.size();
			sparse.resize(index + 1);
			for (int i = old_size; i < sparse.size(); i++) {
				sparse.write[i] = (uint32_t)-1;
			}
		}

		if (has(p_entity)) {
			components.write[sparse[index]] = p_component;
			return;
		}

		uint32_t dense_index = dense.size();
		sparse.write[index] = dense_index;
		dense.push_back(p_entity);
		components.push_back(p_component);

		// Native Dispatch perfectly signaling creation directly bypassing manual mapping
		if (on_added_observers.size() > 0) {
			 Variant arg = p_entity;
			 const Variant *argptr = &arg;
			 for (int i = 0; i < on_added_observers.size(); i++) {
				 Callable::CallError err; Variant ret;
				 on_added_observers[i].callp(&argptr, 1, ret, err);
			 }
		}
	}

	void remove(uint64_t p_entity) override {
		if (!has(p_entity)) {
			return;
		}

		// Native Dispatch signaling destruction directly bypassing engine node checks
		if (on_removed_observers.size() > 0) {
			 Variant arg = p_entity;
			 const Variant *argptr = &arg;
			 for (int i = 0; i < on_removed_observers.size(); i++) {
				 Callable::CallError err; Variant ret;
				 on_removed_observers[i].callp(&argptr, 1, ret, err);
			 }
		}

		uint32_t index = get_index(p_entity);
		uint32_t dense_index = sparse[index];
		uint32_t last_dense_index = dense.size() - 1;
		uint64_t last_entity = dense[last_dense_index];

		if (dense_index != last_dense_index) {
			dense.write[dense_index] = last_entity;
			components.write[dense_index] = components[last_dense_index];
			sparse.write[get_index(last_entity)] = dense_index;
		}

		dense.resize(last_dense_index);
		components.resize(last_dense_index);
		sparse.write[index] = (uint32_t)-1;
	}

	bool has(uint64_t p_entity) const override {
		uint32_t index = get_index(p_entity);
		return index < (uint32_t)sparse.size() && sparse[index] != (uint32_t)-1 && sparse[index] < (uint32_t)dense.size() && dense[sparse[index]] == p_entity;
	}

	T &get(uint64_t p_entity) {
		return components.write[sparse[get_index(p_entity)]];
	}

	int size() const override { return dense.size(); }
	const Vector<uint64_t>& get_dense_raw() const override { return dense; }
	Vector<T>& get_components() { return components; }
};
