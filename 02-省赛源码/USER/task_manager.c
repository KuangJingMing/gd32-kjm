/*
 * task_manager.c
 *
 * created: 4/1/2025
 *  author: 
 */

#include "task_manager.h"
#include "main.h"

// Enhanced start_task function with comprehensive error handling
bool start_task(TaskFunction_t taskFunction, const char *taskName, UBaseType_t priority, TaskHandle_t *taskHandle) {
    if (taskFunction == NULL) {
        LOG_E("Task function is NULL");
        return false;
    }

    if (taskName == NULL) {
        LOG_E("Task name is NULL");
        return false;
    }

    if (taskHandle == NULL) {
        LOG_E("Task handle pointer is NULL");
        return false;
    }

    if (priority >= configMAX_PRIORITIES) {
        LOG_E("Priority %u is invalid for task %s. Max priority is %u", (int)priority, taskName, (unsigned int)priority);
        return false;
    }

    if (*taskHandle == NULL) {
        // Create a new task
        BaseType_t result = xTaskCreate(taskFunction, taskName, configMINIMAL_STACK_SIZE * 2, NULL, priority, taskHandle);
        if (result == pdPASS) {
            LOG_I("Task %s created successfully with handle %p", taskName, (void*)*taskHandle);
            // Optionally, check if the task started correctly
            vTaskPrioritySet(*taskHandle, priority);
            return true;
        } else if (result == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY) {
            LOG_E("Failed to create task %s: insufficient memory", taskName);
        } else {
            LOG_E("Failed to create task %s: error code %d", taskName, (int)result);
        }
        *taskHandle = NULL; // Ensure handle is reset
        return false;
    } else {
        // Check the current state of the task
        eTaskState taskState = eTaskGetState(*taskHandle);
        switch (taskState) {
            case eRunning:
                LOG_I("Task %s is already running with handle %p", taskName, (void*)*taskHandle);
                return false;
            case eReady:
                LOG_I("Task %s is ready to run with handle %p", taskName, (void*)*taskHandle);
                return false;
            case eBlocked:
                LOG_I("Task %s is blocked and cannot be resumed with handle %p", taskName, (void*)*taskHandle);
                return false;
            case eSuspended:
                vTaskResume(*taskHandle);
                LOG_I("Task %s resumed successfully with handle %p", taskName, (void*)*taskHandle);
                return true;
            case eDeleted:
                LOG_E("Task %s is deleted but handle %p was not cleared. Resetting handle.", taskName, (void*)*taskHandle);
                *taskHandle = NULL;
                return false;
            default:
                LOG_E("Task %s is in an unknown state (%d) with handle %p", taskName, taskState, (void*)*taskHandle);
                return false;
        }
    }
}

// Enhanced stop_task function with comprehensive error handling
void stop_task(TaskHandle_t *taskHandle, const char *taskName, void (*releaseResources)(void)) {
    if (taskHandle == NULL) {
        LOG_E("Task handle pointer is NULL for task %s", taskName ? taskName : "Unknown");
        return;
    }

    if (taskName == NULL) {
        LOG_E("Task name is NULL while stopping task");
        return;
    }

    if (*taskHandle != NULL) {
        eTaskState taskState = eTaskGetState(*taskHandle);
        if (taskState != eDeleted) {
            // Suspend the task before deletion
            if (taskState != eSuspended) {
                vTaskSuspend(*taskHandle);
                LOG_I("Task %s suspended", taskName);
            } else {
                LOG_I("Task %s is already suspended", taskName);
            }

            // Release resources
            if (releaseResources != NULL) {
                releaseResources();
                LOG_I("Resources released for task %s", taskName);
            } else {
                LOG_E("No resource release function provided for task %s", taskName);
            }

            // Delete the task
            vTaskDelete(*taskHandle);
            LOG_I("Task %s deleted successfully", taskName);

            // Ensure handle is reset
            *taskHandle = NULL;
        } else {
            LOG_E("Task %s is already deleted", taskName);
            *taskHandle = NULL; // Reset handle if task is deleted
        }
    } else {
        LOG_E("Task %s handle is NULL or task is not running", taskName);
    }
}
