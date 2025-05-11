#ifndef SNR991X_SYSTEM_EPT_H
#define SNR991X_SYSTEM_EPT_H



/**
 * @brief 通信模块划分的枚举类型
 *          一般情况下，只有需要相应mailbox中断服务的模块，才使用一个类型
 * 
 */
typedef enum
{
    decoder_inner_serve_ept_num = 0,//
    // vad_fe_inner_serve_ept_num,
    // dnn_inner_serve_ept_num,
    asr_top_inner_serve_ept_num,
    asr_top_outside_serve_ept_num,
    flash_manage_serve_inner_ept_num,
    flash_manage_serve_outside_ept_num,
    flash_control_serve_inner_ept_num,
    codec_manage_serve_inner_ept_num,
    audio_in_serve_ept_num,
    other_func_inner_serve_ept_num,
    serve_ept_end,
}mailbox_serve_ept_t;


/**
 * @brief 这个枚举类型是每个请求另外一个核处理的函数的编号，同一个模块内部的函数编号唯一即可
 * 
 */
typedef enum
{
    /*decoder模块定义的函数编号*/
    decoder_rpmsg_port_num_start = 0,

    open_asr_detail_result_cal_ept_num,

    get_asr_sys_verinfo_cal_ept_num,

    send_startup_msg_to_decoder_cal_ept_num,

    wait_decoder_createmodel_done_cal_ept_num,

    send_create_model_msg_to_decoder_cal_ept_num,

    set_model_parameter_info_cfg_lg_model_info_cal_ept_num,

    set_model_parameter_info_cfg_ac_info_cal_ept_num,

    set_model_parameter_info_startup_index_cal_ept_num,

    model_parameter_info_clear_cal_ept_num,

    dynmic_confidence_config_cal_ept_num,

    dynmic_confidence_en_cfg_cal_ept_num,

    decoder_split_init_cal_ept_num,

    get_decoded_slice_nums_cal_ept_num,

    wait_decoder_task_vadfe_clear_done_cal_ept_num,

    send_vadfe_cleardone_msg_to_decoder_cal_ept_num,

    send_dnn_cleardone_msg_to_decoder_cal_ept_num,

    send_dnn_mtxdone_msg_to_decoder_cal_ept_num,

    get_decode_baseaddr_curframes_cal_ept_num,

    nn_get_decoding_state_cal_ept_num,

    get_decoder_manage_is_decodeing_state_addr_cal_ept_num,

    get_g_prun_count_max_addr_cal_ept_num,

    get_decode_cur_asrresult_isgood_ept_num,
    

    decoder_rpmsg_port_num_end,

    /*vad_fe模块定义的函数编号*/
    vad_fe_rpmsg_port_num_start,

    cmvn_update_mean_var_config_cal_ept_num,

    cmvn_update_weight_config_cal_ept_num,

    send_vadcfg_vadend_frames_msg_cal_ept_num,
    jd_asr_rlt_good_ept_num,

    vad_fe_rpmsg_port_num_end,


    /*dnn模块定义的函数编号*/
    dnn_rpmsg_port_num_start,

    get_dnn_version_ept_num,

    set_g_asrtop_cursegment_decoderinbuf_nums_cal_ept_num,

    get_g_asrtop_cursegment_decoderinbuf_nums_cal_ept_num,

    get_g_prun_count_cal_ept_num,

    get_g_prun_tab_cal_ept_num,

    get_dnn_asr_skpnums_cal_ept_num,

    set_g_prun_count_cal_ept_num,

    set_g_dnn_output2_enable_cal_ept_num,

    get_g_dnn_asr_acmodel_inflash_flag_cal_ept_num,

    get_g_prun_count_addr_cal_ept_num,

    set_g_dnn_asr_acmodel_addr_cal_ept_num,

    set_g_dnn_asr_acmodel_inflash_flag_cal_ept_num,

    dnn_output2_able_state_cal_ept_num,

    get_dnn_fe_ready_nums_cal_ept_num,

    check_fe_nums_isready_cal_ept_num,

    send_dnn_run_msg_cal_ept_num,

    send_clear_msg_to_dnn_cal_ept_num,

    wait_dnn_task_clear_done_cal_ept_num,

    get_dnn_outbuf_nums_cal_ept_num,

    get_dnn_each_buf_slice_count_cal_ept_num,

    get_dnn_mtx_slices_cal_ept_num,

    check_decoder_is_overwritten_cal_ept_num,

    send_lastdnn_run_msg_cal_ept_num,

    dnn_split_init_cal_ept_num,

    send_suspend_msg_to_dnn_cal_ept_num,

    wait_dnn_suspend_done_cal_ept_num,

    send_config_msg_to_dnn_cal_ept_num,

    wait_dnn_config_done_cal_ept_num,

    send_resume_msg_to_dnn_cal_ept_num,

    send_startup_msg_to_dnn_cal_ept_num,

    get_g_asrtop_dnnoutbuf_start_addr_cal_ept_num,

    get_g_asrtop_dnnoutbuf_size_cal_ept_num,

    set_nn_to_dtw_init_info_ept_num,

    dnn_rpmsg_port_num_end,

    /*asr_top模块定义的函数编号*/
    asr_top_rpmsg_port_num_start,

    set_g_asrtop_asrsys_clear_flag_cal_ept_num,

    send_asrsys_clear_msg_cal_ept_num,

    get_g_asrtop_asrsys_clear_flag_cal_ept_num,

    dy_adj_cmpt_callback_cal_ept_num,

    asrtop_asr_system_continue_ept_num,
    asrtop_asr_system_continue_done_isr_ept_num,

    asrtop_asr_system_pause_ept_num,
    asrtop_asr_system_pause_done_isr_ept_num,

    asrtop_asr_system_create_model_ept_num,
    asrtop_asr_system_create_model_done_isr_ept_num,

    asr_top_rpmsg_port_num_end,//偶数


    /*flash_manage模块定义的函数编号*/
    flash_manage_nuclear_com_num_start,

    post_read_flash_ept_num,
    post_read_flash_op_done_ept_num,

    post_erase_flash_ept_num,
    post_erase_flash_op_done_ept_num,

    post_write_flash_ept_num,
    post_write_flash_op_done_ept_num,

    post_spic_read_unique_id_ept_num,
    post_spic_read_unique_id_op_done_ept_num,

    is_flash_power_off_ept_num,

    flash_manage_nuclear_com_num_end,


    /*flash_control模块定义的函数编号*/
    flash_control_nuclear_com_num_start,

    get_current_model_addr_ept_num,

    is_ci_flash_data_info_inited_ept_num,


    flash_control_nuclear_com_num_end,


    /*codec_manage模块定义的函数编号*/
    codec_manage_nuclear_com_num_start,

    cm_write_codec_ept_num,
    cm_get_pcm_buffer_ept_num,
    cm_release_pcm_buffer_ept_num,
    cm_config_pcm_buffer_ept_num,
    cm_config_codec_ept_num,
    cm_start_codec_ept_num,
    cm_set_codec_mute_ept_num,
    cm_stop_codec_ept_num,
    cm_get_codec_empty_buffer_number_ept_num,
    cm_get_codec_busy_buffer_number_ept_num,
    audio_pre_rslt_write_data_ept_num,
    cm_set_codec_alc_ept_num,
    cm_set_codec_adc_gain_ept_num,

    cinn_malloc_in_host_sram_ept_num,
    cinn_free_in_host_sram_ept_num,
    

    codec_manage_nuclear_com_num_end,

    audio_in_nuclear_com_num_start,
    send_voice_info_to_audio_in_manage_ept_num,
    set_ssp_registe_ept_num,
    audio_in_nuclear_com_num_end,


    other_fun_nuclear_com_num_start,
    ciss_init_ept_num,
    other_fun_nuclear_com_num_end,
}asr_rpmsg_ept_num_t;

#endif /* SNR991X_SYSTEM_EPT_H */



