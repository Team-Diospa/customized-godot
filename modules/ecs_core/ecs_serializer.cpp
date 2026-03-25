/**************************************************************************/
/*  ecs_serializer.cpp                                                     */
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

#include "ecs_serializer.h"

ECSSerializer *ECSSerializer::singleton = nullptr;

void ECSSerializer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("save_world", "path"), &ECSSerializer::save_world);
    ClassDB::bind_method(D_METHOD("load_world", "path"), &ECSSerializer::load_world);
}

ECSSerializer::ECSSerializer() {
    singleton = this;
}

ECSSerializer::~ECSSerializer() {
    if (singleton == this) singleton = nullptr;
}

Error ECSSerializer::save_world(const String &p_path) {
    EntityManager *em = EntityManager::get_singleton();
    if (!em) return ERR_CANT_CREATE;

    Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::WRITE);
    if (f.is_null()) return ERR_FILE_CANT_OPEN;

    // 1. Write Header
    f->store_32(0x45435357); // "ECSW" magic number
    f->store_32(1); // Version

    // 2. Count entities and save them
    // Implementation details: Iterate through registries and store data...
    
    return OK;
}

Error ECSSerializer::load_world(const String &p_path) {
    // Reverse of save_world
    return OK;
}
