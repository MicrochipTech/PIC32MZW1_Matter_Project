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
#define KVS_VALUE_SIZE  320
#define MAX_KEY_NUM     60
#define KEY_NUM_PER_PAGE    10
#define NUM_OF_PAGE_KVS     MAX_KEY_NUM / KEY_NUM_PER_PAGE    
#define KVS_KEY_ADDRESS 0x900FA000//0x900FC000
#define KVS_VALUE_ADDRESS 0x900FF000

static SemaphoreHandle_t storage_mutex = NULL;

typedef struct
{
    char key[KVS_KEY_SIZE];
    int key_len;
    char value[KVS_VALUE_SIZE];
    int value_len;
    
}KVS_DATA_FRAME;

KVS_DATA_FRAME  dvs_data[KEY_NUM_PER_PAGE] CACHE_ALIGN;

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
    int i, j;
    uint8_t *writePtr;
    uint32_t base_addr;
    ChipLogProgress(DeviceLayer, "KeyValueStoreManagerImpl:: Init...");
    
    if (storage_mutex == NULL)
    	storage_mutex = xSemaphoreCreateMutex();    
    
    NVM_CallbackRegister(event_handler, (uintptr_t)NULL);

    for (i=0; i<NUM_OF_PAGE_KVS; i++)
    {
        xSemaphoreTake(storage_mutex, portMAX_DELAY); 
        
        base_addr = KVS_KEY_ADDRESS + i*NVM_FLASH_PAGESIZE;

        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), base_addr);

        if (dvs_data[0].key_len < 0)
        {
            while(NVM_IsBusy() == true);
            NVM_PageErase(base_addr);
            while(xferDone == false);

            memset(dvs_data,0, sizeof(dvs_data));
            xferDone = false;
            writePtr = (uint8_t *) &dvs_data;

            for (j=0; j<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; j++)
            {
                xferDone = false;
                while(NVM_IsBusy() == true);
                NVM_RowWrite( (uint32_t*) writePtr, base_addr + j*NVM_FLASH_ROWSIZE  );
                while(xferDone == false);

                writePtr += NVM_FLASH_ROWSIZE;
            }
        }

        xSemaphoreGive(storage_mutex);
    }

    return CHIP_NO_ERROR;
}



CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
{
    //ChipLogProgress(DeviceLayer, "_Get... key = %s, value_size = %d", key, value_size);
    
    bool found = 0;
    int i,j;
    uint32_t base_addr;
    
    for (i=0; i<NUM_OF_PAGE_KVS; i++)
    {
        xSemaphoreTake(storage_mutex, portMAX_DELAY); 
        
        base_addr = KVS_KEY_ADDRESS + i*NVM_FLASH_PAGESIZE;

        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), base_addr);
        
        xSemaphoreGive(storage_mutex);  
        
        for (j=0; j< KEY_NUM_PER_PAGE; j++)
        {
            if (0 == strcmp(dvs_data[j].key, key))
            {
                found = 1;
                memcpy((char *)value, dvs_data[j].value, value_size);
                if (read_bytes_size != NULL)
                    *read_bytes_size = dvs_data[j].value_len;
                break;

            }
        }
        
        if (found == 1)
            break;
    }
    
    if (found == 0)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    else
        return CHIP_NO_ERROR;
           
}


CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    //ChipLogProgress(DeviceLayer, " _Put... key = %s, value_size = %d", key, value_size);  
    bool found = 0;
    bool is_put = 0;
    int i,j;
    uint8_t *writePtr;
    uint32_t base_addr;
    
    for (i=0; i<NUM_OF_PAGE_KVS; i++)
    {
        xSemaphoreTake(storage_mutex, portMAX_DELAY); 
        
        base_addr = KVS_KEY_ADDRESS + i*NVM_FLASH_PAGESIZE;

        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), base_addr);
        
        xSemaphoreGive(storage_mutex);  
        
        for (j=0; j< KEY_NUM_PER_PAGE; j++)
        {
            if (0 == strcmp(dvs_data[j].key, key))
            {
                ChipLogProgress(DeviceLayer, "KeyValueStoreManagerImpl::_Put, done.."); 
                found = 1;
                is_put = 1;
                memset(dvs_data[j].value, 0, KVS_VALUE_SIZE);
                memcpy(dvs_data[j].value, value, value_size);
                dvs_data[j].value_len = value_size;
                break;

            }
        }
        
        if (is_put)
        {
            xSemaphoreTake(storage_mutex, portMAX_DELAY); 

            // erase before write
            xferDone = false;
            while(NVM_IsBusy() == true);
            NVM_PageErase(base_addr);
            while(xferDone == false);
            
            // write data to nvm
            writePtr = (uint8_t *) &dvs_data;
            for (j=0; j<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; j++)
            {
                xferDone = false;
                while(NVM_IsBusy() == true);
                NVM_RowWrite( (uint32_t*) writePtr, base_addr + j*NVM_FLASH_ROWSIZE  );
                while(xferDone == false);

                writePtr += NVM_FLASH_ROWSIZE;
            }
            xSemaphoreGive(storage_mutex);  
            break;
    
        }
    }
    if (found == 0)
    {
        for (i=0; i<NUM_OF_PAGE_KVS; i++)
        {
            xSemaphoreTake(storage_mutex, portMAX_DELAY); 

            base_addr = KVS_KEY_ADDRESS + i*NVM_FLASH_PAGESIZE;

            xferDone = false;
            while(NVM_IsBusy() == true);
            NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), base_addr);

            xSemaphoreGive(storage_mutex);  

            for (j=0; j< KEY_NUM_PER_PAGE; j++)
            {
                if (dvs_data[j].key_len == 0)
                {
                    ChipLogProgress(DeviceLayer, "KeyValueStoreManagerImpl::_Put, %d %d done..", i, j); 
                    is_put = 1; 
                    strcpy(dvs_data[j].key, key);
                    dvs_data[j].key_len = strlen(key);

                    if (strlen(key) > 64)
                         ChipLogProgress(DeviceLayer, "Key size is too large.. %d", strlen(key));
                    if (value_size > 320)
                         ChipLogProgress(DeviceLayer, "Value size is too large.. %d", value_size);

                    memcpy(dvs_data[j].value, value, value_size);
                    dvs_data[j].value_len = value_size;
                    
                    break;
                }
            }
            if (is_put)
            {
                xSemaphoreTake(storage_mutex, portMAX_DELAY); 

                // erase before write
                xferDone = false;
                while(NVM_IsBusy() == true);
                NVM_PageErase(base_addr);
                while(xferDone == false);

                // write data to nvm
                writePtr = (uint8_t *) &dvs_data;
                for (j=0; j<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; j++)
                {
                    xferDone = false;
                    while(NVM_IsBusy() == true);
                    NVM_RowWrite( (uint32_t*) writePtr, base_addr + j*NVM_FLASH_ROWSIZE  );
                    while(xferDone == false);

                    writePtr += NVM_FLASH_ROWSIZE;
                }
                xSemaphoreGive(storage_mutex);
                break;
            }
        }
    }
    if (!is_put)
        ChipLogProgress(DeviceLayer, "KeyValueStoreManagerImpl::_Put, storage is full..");
    
    return CHIP_NO_ERROR; 
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key) 
{ 
    //ChipLogProgress(DeviceLayer, "ToDo: eyValueStoreManagerImpl:: _Delete... key = %s", key);

    bool found = 0;
    int i,j;
    uint8_t *writePtr;
    uint32_t base_addr;
    
    for (i=0; i<NUM_OF_PAGE_KVS; i++)
    {
        xSemaphoreTake(storage_mutex, portMAX_DELAY); 
        
        base_addr = KVS_KEY_ADDRESS + i*NVM_FLASH_PAGESIZE;

        xferDone = false;
        while(NVM_IsBusy() == true);
        NVM_Read( (uint32_t *) dvs_data, sizeof(dvs_data), base_addr);
        
        xSemaphoreGive(storage_mutex);  
        
        for (i=0; i< KEY_NUM_PER_PAGE; i++)
        {
            if (0 == strcmp(dvs_data[i].key, key))
            {
                ChipLogProgress(DeviceLayer, "KeyValueStoreManagerImpl::_Delete, done.."); 
                found = 1;
                memset(&dvs_data[i],0, sizeof(dvs_data[0]));
                break;
            }
        }
        
        if (found)
        {
            xSemaphoreTake(storage_mutex, portMAX_DELAY); 

            // erase before write
            xferDone = false;
            while(NVM_IsBusy() == true);
            NVM_PageErase(base_addr);
            while(xferDone == false);
            
            // write data to nvm
            writePtr = (uint8_t *) &dvs_data;
            for (j=0; j<=(sizeof(dvs_data))/NVM_FLASH_ROWSIZE; j++)
            {
                xferDone = false;
                while(NVM_IsBusy() == true);
                NVM_RowWrite( (uint32_t*) writePtr, base_addr + j*NVM_FLASH_ROWSIZE  );
                while(xferDone == false);

                writePtr += NVM_FLASH_ROWSIZE;
            }
            xSemaphoreGive(storage_mutex);  
            break;
    
        }
    }
    
    if (found == 0)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        
    }    
 
    return CHIP_NO_ERROR;
}


CHIP_ERROR KeyValueStoreManagerImpl::Erase(void)
{
    int i;
    uint32_t base_addr;
    
    for (i=0; i<NUM_OF_PAGE_KVS; i++)
    {
        base_addr = KVS_KEY_ADDRESS + i*NVM_FLASH_PAGESIZE;
        xSemaphoreTake(storage_mutex, portMAX_DELAY); 

            // erase before write
            xferDone = false;
            while(NVM_IsBusy() == true);
            NVM_PageErase(base_addr);
            while(xferDone == false);
            
        xSemaphoreGive(storage_mutex);     
    }
 
    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
