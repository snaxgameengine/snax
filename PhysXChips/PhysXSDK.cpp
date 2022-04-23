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
#include "PhysXSDK.h"


using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(PhysXSDK, MTEXT("PhysX SDK"), PHYSXSDK_GUID, CHIP_GUID);


class Allocer : public PxAllocatorCallback
{
        void* allocate(size_t size, const Char *file, const char*, int32 line)
        {
#if defined(DEBUG) || defined(_DEBUG)
            return mm().getDebug(size, file, line);// mmalloc(size);
#else
            return mm().get(size);
#endif
        }

        void deallocate(void* ptr)
        {
#if defined(DEBUG) || defined(_DEBUG)
			mm().freeDebug(ptr);
#else
			mm().free(ptr);
#endif
        }
};
//static Allocer gDefaultAllocatorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;  // Note using default allocator to avoid 16-byte alignement issues for x86. TODO: fix it!

class Errors : public PxErrorCallback
{
public:
        virtual void reportError(PxErrorCode::Enum code, const Char* message, const Char* file, int32 line)
		{
			MessageSeverity sev = INFO;
			const Char* errorCode = NULL;

			switch (code)
			{
			case PxErrorCode::eNO_ERROR:
				sev = INFO;
				errorCode = "No Error";
			case PxErrorCode::eDEBUG_INFO:
				sev = DINFO;
				errorCode = "Info";
				break;
			case PxErrorCode::eDEBUG_WARNING:
				sev = WARN;
				errorCode = "Warning";
				break;
			case PxErrorCode::eINVALID_PARAMETER:
				sev = FATAL;
				errorCode = "Invalid Parameter";
				break;
			case PxErrorCode::eINVALID_OPERATION:
				sev = FATAL;
				errorCode = "Invalid Operation";
				break;
			case PxErrorCode::eOUT_OF_MEMORY:
				sev = FATAL;
				errorCode = "Out of Memory";
				break;
			case PxErrorCode::eINTERNAL_ERROR:
				sev = FATAL;
				errorCode = "Internal Error";
				break;
			case PxErrorCode::eABORT:
				sev = FATAL;
				errorCode = "Abort";
				break;
			case PxErrorCode::ePERF_WARNING:
				sev = NOTICE;
				errorCode = "Performance Warning";
				break;
			default:
				errorCode = "Unknown Error";
				break;
			}

			msg(sev, strUtils::format("%s: %s  [%s (%d)]", errorCode, message, file, line));
	}
};
static Errors gDefaultErrorCallback;

PhysXSDK::PhysXSDK()
{
#if defined(DEBUG) || defined(_DEBUG)
	bool recordMemoryAllocations = true;
#else
	bool recordMemoryAllocations = false;
#endif

	_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);

//	_pzm = &PxProfileZoneManager::createProfileZoneManager(_foundation);
	
	_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, _toleranceScale, recordMemoryAllocations, nullptr);

	_extensionsOK = PxInitExtensions(*_physics, nullptr);

	_defaultMaterial = nullptr;
	_cooking = nullptr;
	_cudaContextManager = nullptr;
}

PhysXSDK::~PhysXSDK()
{
	PX_RELEASE(_cudaContextManager);
	PX_RELEASE(_cooking);
	PX_RELEASE(_defaultMaterial);

	if (_extensionsOK)
		PxCloseExtensions();

	PX_RELEASE(_physics);

//	if (_pzm)
//		_pzm->release();
	PX_RELEASE(_foundation);
}

PxMaterial *PhysXSDK::GetDefaultMaterial()
{
	if (!_defaultMaterial)
		_defaultMaterial = _physics->createMaterial(0.5f, 0.5f, 0.1f);

	return _defaultMaterial;
}

PxCooking *PhysXSDK::GetCooking()
{
	if (!_cooking)
		_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *_foundation, PxCookingParams(_toleranceScale));
	return _cooking;
}

PxCudaContextManager *PhysXSDK::GetCudaContextManager()
{
	if (_cudaContextManager && _cudaContextManager->contextIsValid())
		return _cudaContextManager;

//	int32 gg =PxGetSuggestedCudaDeviceOrdinal(gDefaultErrorCallback);

	PxCudaContextManagerDesc cudaContextManagerDesc;
//	cudaContextManagerDesc.graphicsDevice = ((D3DGraphics*)engine->GetGraphics())->GetDevice();
//	cudaContextManagerDesc.interopMode = pxtask::CudaInteropMode::D3D11_INTEROP;
	_cudaContextManager = PxCreateCudaContextManager(*_foundation, cudaContextManagerDesc);
	if (!_cudaContextManager)
		return nullptr;

	return _cudaContextManager->contextIsValid() ? _cudaContextManager : nullptr;
}


PhysXUsage::PhysXUsage() : _sdk(nullptr)
{
}

PhysXUsage::~PhysXUsage()
{
}


PhysXSDK *PhysXUsage::sdk() const
{
	if (!_sdk)
		_sdk = (PhysXSDK*)engine->GetChipManager()->GetGlobalChip(PHYSXSDK_GUID);
	return _sdk;
}

PxPhysics *PhysXUsage::physics() const
{
	PhysXSDK *p = sdk();
	if (p)
		return p->GetPhysics();
	return nullptr;
}