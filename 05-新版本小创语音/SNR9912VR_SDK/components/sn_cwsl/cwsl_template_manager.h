#ifndef __CWSL_TEMPLATE_MANAGER_H__
#define __CWSL_TEMPLATE_MANAGER_H__

#include "cwsl_manage.h"

#pragma pack(push)
#pragma pack(4)
typedef struct
{
    cwsl_word_type_t word_type;             // 是否唤醒词
    uint8_t group_id;                       // 模板分组号
    uint16_t frame_num;                     // 保存的特征帧数
    uint32_t cmd_id;                        // 模板对应的ASR命令词ID
}tpl_info_t;
#pragma pack(pop)

uint32_t cwsl_tm_init(uint32_t max_tpl_number, uint32_t max_tpl_size);

uint32_t cwsl_tm_del_by_index(uint16_t index);

int cwsl_tm_get_tpl_index(uint16_t group_id, uint32_t cmd_id);

uint32_t cwsl_tm_get_tpl_cmd_id_by_index(int index);

uint32_t cwsl_tm_del_by_cmd_id(uint16_t group, uint32_t cmd_id, cwsl_word_type_t word_type);

int cwsl_tm_get_words_index(uint8_t * index_buffer, int buffer_length, uint16_t group, cwsl_word_type_t word_type);

int cwsl_tm_get_reg_tpl_number(cwsl_word_type_t word_type);

int cwsl_tm_get_left_tpl_number();

int cwsl_tm_get_max_tpl_number();




#endif  //__CWSL_TEMPLATE_MANAGER_H__

