// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#ifdef Primitives_EXPORTS
#define PRIMITIVES_API __declspec(dllexport)
#else
#define PRIMITIVES_API __declspec(dllimport)
#endif

// This class is exported from the Primitives.dll
class PRIMITIVES_API CPrimitives {
public:
	CPrimitives(void);
	// TODO: add your methods here.
};

extern PRIMITIVES_API int nPrimitives;

PRIMITIVES_API int fnPrimitives(void);
