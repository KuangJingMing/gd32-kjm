#ifndef __USER_MSG_DEAL_H__
#define __USER_MSG_DEAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 按命令词ID响应asr消息处理
 * 
 * @param asr_msg 
 * @param cmd_handle 
 * @param cmd_id 
 * @return uint32_t 
 */
uint32_t deal_asr_msg_by_cmd_id(sys_msg_asr_data_t *asr_msg, cmd_handle_t cmd_handle, uint16_t cmd_id);

/**
 * @brief 按语义ID响应asr消息处理
 * 
 * @param asr_msg 
 * @param cmd_handle 
 * @param semantic_id 
 * @return uint32_t 
 */
uint32_t deal_asr_msg_by_semantic_id(sys_msg_asr_data_t *asr_msg, cmd_handle_t cmd_handle, uint32_t semantic_id);

/**
 * @brief 用户自定义消息处理
 * 
 * @param msg 
 * @return uint32_t 
 */
uint32_t deal_userdef_msg(sys_msg_t *msg);

/**
 * @brief 按键消息处理
 * 
 * @param msg 
 * @return uint32_t 
 */
void userapp_deal_key_msg(sys_msg_key_data_t  *key_msg);
void UserMain(void* p);

#ifdef __cplusplus
}
#endif

#endif

