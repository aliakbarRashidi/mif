//-------------------------------------------------------------------
//  MetaInfo Framework (MIF)
//  https://github.com/tdv/mif
//  Created:     04.2017
//  Copyright (C) 2016-2017 tdv
//-------------------------------------------------------------------

#ifndef __MIF_APPLICATION_ID_SERVICE_H__
#define __MIF_APPLICATION_ID_SERVICE_H__

// MIF
#include "mif/common/crc32.h"

namespace Mif
{
    namespace Application
    {
        namespace Id
        {
            namespace Service
            {

                enum
                {
                    ComponentsFactory = Common::Crc32("Mif.Application.Service.ComponentsFactory")
                };

            }   // namespace Service
        }   // namespace Id
    }   // namespace Application
}   // namespace Mif

#endif  // !__MIF_APPLICATION_ID_SERVICE_H__
