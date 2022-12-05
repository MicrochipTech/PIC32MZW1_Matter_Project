/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <platform/KeyValueStoreManager.h>
#include <lib/support/logging/CHIPLogging.h>

#include "definitions.h"

#define KVS_KEY_SIZE    64
#define KVS_VALUE_SIZE  280
#define MAX_KEY_NUM     30
#define KVS_KEY_ADDRESS 0x900FD000
#define KVS_VALUE_ADDRESS 0x900FF000

static SemaphoreHandle_t storage_mutex = NULL;

typedef struct
{
    char key[KVS_KEY_SIZE];
    int key_len;
    char value[KVS_VALUE_SIZE];
    int value_len;
    
}KVS_DATA_FRAME;

KVS_DATA_FRAME  dvs_data[MAX_KEY_NUM] CACHE_ALIGN;
//KVS_DATA_FRAME  dvs_data_read[MAX_KEY_NUM];

static volatile bool xferDone = false;

extern "C" {
    
static void event_handler(uintptr_t context)
{
    xferDone = true;
}

}

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

/** Singleton instance of the KeyValueStoreManager implementation object.
 */
KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;


CHIP_ERROR KeyValueStoreManagerImpl::Init()
{
    int i;
    ChipLogProgress(DeviceLayer, "KeyValueStoreManagerImpl:: Init...");
    
    if (storage_mutex == NULL)
    	storage_mutex = xSemaphoreCreateMutex();    
    
    NVM_CallbackRegister(event_handler, (uintptr_t)NULL);

    xSemaphoreTake(storage_mutex, portMAX_DELAY); 
    
    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), KVS_KEY_ADDRESS);

    xSemaphoreGive(storage_mutex);

    if (dvs_data[0].key_len < 0)
    {
        ChipLogProgress(DeviceLayer, "reinit nvm..");

        xSemaphoreTake(storage_mutex, portMAX_DELAY); 
        while(NVM_IsBusy() == true);
        NVM_PageErase(KVS_KEY_ADDRESS);
        while(xferDone == false);

        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_PageErase(KVS_KEY_ADDRESS + NVM_FLASH_PAGESIZE);
        while(xferDone == false);
        
        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_PageErase(KVS_KEY_ADDRESS + 2*NVM_FLASH_PAGESIZE);
        while(xferDone == false);

        memset(dvs_data,0, sizeof(dvs_data));
        //dvs_data[0].key_len = 2;
        //dvs_data[19].key_len = 33;
        xferDone = false;

        uint8_t *writePtr = (uint8_t *) &dvs_data;
        
        for (i=0; i<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; i++)
        {
            printf(" ptr %d = 0x%p \r\n", i, writePtr);
            xferDone = false;
            while(NVM_IsBusy() == true);
            NVM_RowWrite( (uint32_t*) writePtr, KVS_KEY_ADDRESS + i*NVM_FLASH_ROWSIZE  );
            while(xferDone == false);

            writePtr += NVM_FLASH_ROWSIZE;
        }
        xSemaphoreGive(storage_mutex); 

    }


    // for test only
#if 0   
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS);
    while(xferDone == false);

    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS + NVM_FLASH_PAGESIZE);
    while(xferDone == false);
    
    
    
    dvs_data[0].key_len = 567;
    dvs_data[19].key_len = 899;
    xferDone = false;

    writePtr = (uint8_t *) &dvs_data;
    

    for (i=0; i<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; i++)
    {
        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_RowWrite( (uint32_t*) writePtr, KVS_KEY_ADDRESS + i*NVM_FLASH_ROWSIZE  );
        while(xferDone == false);

        writePtr += NVM_FLASH_ROWSIZE;
    }

    memset(dvs_data,0, sizeof(dvs_data));
    while(NVM_IsBusy() == true);
    NVM_Read( (uint32_t *) &dvs_data, sizeof(dvs_data), KVS_KEY_ADDRESS);
    printf("get key_len3 = %d, size = %d\r\n", dvs_data[0].key_len, sizeof(dvs_data));
    printf("get key_len4 = %d, size = %d\r\n", dvs_data[19].key_len, sizeof(dvs_data));
    
    
#endif   
    

    return CHIP_NO_ERROR;
}



CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
{
    //ChipLogProgress(DeviceLayer, "_Get... key = %s, value_size = %d", key, value_size);
    
    bool found = 0;
    int i;
    
    xSemaphoreTake(storage_mutex, portMAX_DELAY); 
    
    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), KVS_KEY_ADDRESS);

    xSemaphoreGive(storage_mutex);    
    
    
    for (i=0; i< MAX_KEY_NUM; i++)
    {
        if (0 == strcmp(dvs_data[i].key, key))
        {
            found = 1;
            memcpy((char *)value, dvs_data[i].value, value_size);
            if (read_bytes_size != NULL)
                *read_bytes_size = dvs_data[i].value_len;
            break;
            
        }
    }

    if (found == 0)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    else
        return CHIP_NO_ERROR;
    

    return CHIP_NO_ERROR;
   
}


CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    //ChipLogProgress(DeviceLayer, " _Put... key = %s, value_size = %d", key, value_size);
   
    
    bool found = 0;
    int i;
    
    xSemaphoreTake(storage_mutex, portMAX_DELAY); 
    
    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), KVS_KEY_ADDRESS);
    
    xSemaphoreGive(storage_mutex);    
    
    
    for (i=0; i< MAX_KEY_NUM; i++)
    {
        if (0 == strcmp(dvs_data[i].key, key))
        {
            found = 1;
            //memcpy((char *)value, dvs_data[i].value, value_size);
            memset(dvs_data[i].value, 0, KVS_VALUE_SIZE);
            memcpy(dvs_data[i].value, value, value_size);
            dvs_data[i].value_len = value_size;
            break;
            
        }
    }
    
    if (found == 0)
    {
        for (i=0; i< MAX_KEY_NUM; i++)
        {
            if (dvs_data[i].key_len == 0)
            {              
                strcpy(dvs_data[i].key, key);
                dvs_data[i].key_len = strlen(key);
                memcpy(dvs_data[i].value, value, value_size);
                dvs_data[i].value_len = value_size;
                
                break;

            }
        }
    }    
    
    
    xSemaphoreTake(storage_mutex, portMAX_DELAY); 
    
    // erase before write
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS);
    while(xferDone == false);

    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS + NVM_FLASH_PAGESIZE);
    while(xferDone == false);

    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS + 2*NVM_FLASH_PAGESIZE);
    while(xferDone == false);
    
    
    // write data to nvm
    uint8_t *writePtr = (uint8_t *) &dvs_data;
    for (i=0; i<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; i++)
    {
        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_RowWrite( (uint32_t*) writePtr, KVS_KEY_ADDRESS + i*NVM_FLASH_ROWSIZE  );
        while(xferDone == false);

        writePtr += NVM_FLASH_ROWSIZE;
    }

    xSemaphoreGive(storage_mutex);    
        
    return CHIP_NO_ERROR;
    
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key) 
{ 
    //ChipLogProgress(DeviceLayer, "ToDo: eyValueStoreManagerImpl:: _Delete... key = %s", key);

    bool found = 0;
    int i;
    
    xSemaphoreTake(storage_mutex, portMAX_DELAY); 
    
    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), KVS_KEY_ADDRESS);
    
    xSemaphoreGive(storage_mutex);    
    
    for (i=0; i< MAX_KEY_NUM; i++)
    {
        if (0 == strcmp(dvs_data[i].key, key))
        {
            found = 1;
            memset(&dvs_data[i],0, sizeof(dvs_data[0]));
            break;
            
        }
    }
    
    if (found == 0)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        
    }    
    
    xSemaphoreTake(storage_mutex, portMAX_DELAY); 
    
    // erase before write
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS);
    while(xferDone == false);

    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS + NVM_FLASH_PAGESIZE);
    while(xferDone == false);

    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS + 2*NVM_FLASH_PAGESIZE);
    while(xferDone == false);
    
    
    // write data to nvm
    uint8_t *writePtr = (uint8_t *) &dvs_data;
    for (i=0; i<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; i++)
    {
        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_RowWrite( (uint32_t*) writePtr, KVS_KEY_ADDRESS + i*NVM_FLASH_ROWSIZE  );
        while(xferDone == false);

        writePtr += NVM_FLASH_ROWSIZE;
    }

    xSemaphoreGive(storage_mutex);    

    return CHIP_NO_ERROR;
}


CHIP_ERROR KeyValueStoreManagerImpl::Erase(void)
{
    xSemaphoreTake(storage_mutex, portMAX_DELAY); 
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS);
    while(xferDone == false);

    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS + NVM_FLASH_PAGESIZE);
    while(xferDone == false);

    xferDone = false;
    while(NVM_IsBusy() == true);
    NVM_PageErase(KVS_KEY_ADDRESS + 2*NVM_FLASH_PAGESIZE);
    while(xferDone == false);
    
    xSemaphoreGive(storage_mutex); 
    
    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
