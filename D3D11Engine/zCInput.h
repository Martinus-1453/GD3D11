#pragma once
#include "pch.h"
#include "HookedFunctions.h"

class zCInput {
public:
    int GetDeviceEnabled( int dev ) {
        return reinterpret_cast<int( __fastcall* )( zCInput*, int, int )>
            ( GothicMemoryLocations::zCInput::GetDeviceEnabled )( this, 0, dev );
    }

    void SetDeviceEnabled( int dev, int i ) {
        reinterpret_cast<void( __fastcall* )( zCInput*, int, int, int )>
            ( GothicMemoryLocations::zCInput::SetDeviceEnabled )( this, 0, dev, i );
    }

    inline static zCInput* GetInput() {
        return *(zCInput**)GothicMemoryLocations::GlobalObjects::zCInput;
    };

};
