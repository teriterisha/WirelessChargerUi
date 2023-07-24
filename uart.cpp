#include "uart.h"
#include <QDebug>

unsigned char byte0(short x)
{
    return (x >> 0) & 0xff;
}

unsigned char byte1(short x)
{
    return (x >> 8) & 0xff;
}

unsigned char byte0(int x)
{
    return (x >> 0) & 0xff;
}

unsigned char byte1(int x)
{
    return (x >> 8) & 0xff;
}

unsigned char byte2(int x)
{
    return (x >> 16) & 0xff;
}

unsigned char byte3(int x)
{
    return (x >> 24) & 0xff;
}

UartTranC::UartTranC(int tx_buffer_len)
{
    reserve_len = tx_buffer_len;
    tx_frame_cnt = 5; // 数据内容前的数据量，这个协议有六个byte，所以开始真实写数据的位置为6
    tx_buffer.resize(tx_buffer_len); //预留空间，暂时留了100个，第一次执行发送后会将其长度重新改为自定义发送长度(可能使用同一个发送类发送不同长度的数据时会出错)
}

void UartTranC::uart_frame_tx(unsigned char source_code, unsigned char target_code, unsigned char fun_code,int data_len, const QList<short> &data_tx)
{
    tx_buffer.resize(reserve_len);
    tx_frame_cnt = 5;
    tx_buffer[0] = 0xAB; //协议帧头
    tx_buffer[1] = source_code; //源地址
    tx_buffer[2] = target_code; //目标地址
    tx_buffer[3] = fun_code; //功能码
    tx_buffer[4] = byte0(data_len * 2); //数据长度后八位
    tx_buffer[5] = byte1(data_len * 2); //数据长度前八位
    for(int i = 0; i < data_len; ++i) {
        ++tx_frame_cnt;
        tx_buffer[tx_frame_cnt] = byte0(data_tx[i]);
        ++tx_frame_cnt;
        tx_buffer[tx_frame_cnt] = byte1(data_tx[i]);
    }
    int checkSum = 0;
    int checkSumAdd = 0;
    for(int i = 0; i < tx_frame_cnt + 1; ++i) {
        checkSum += tx_buffer[i];
        checkSumAdd += checkSum;
    }
    ++tx_frame_cnt;
    tx_buffer[tx_frame_cnt] = byte0(checkSum);
    ++tx_frame_cnt;
    tx_buffer[tx_frame_cnt] = byte0(checkSumAdd);
    tx_buffer.resize(data_len * 2 + 8);
}

UartRecC::UartRecC(QObject *parent, int state, int frame_cnt, int frame_len, int frame_copy_len, int bufflen, unsigned char info_header)
    : QObject{parent}
{
    this->state = state;
    this->frame_cnt = frame_cnt;
    this->frame_len = frame_len;
    this->frame_copy_len = frame_copy_len;
    this->info_header = info_header;
    rx_buffer.resize(bufflen);
}

void UartRecC::uart_frame_seg()
{
    if(state == 8){
        // 附加校验和填充
        rx_buffer[7 + frame_cnt] = last_byte;
        frame_copy_len = frame_cnt + 8;
        int checkSum = 0;
        int checkSumAdd = 0;
        for(int i = 0; i < frame_copy_len - 2; ++i){
            checkSum += rx_buffer[i];
            checkSumAdd += checkSum;
        }
//        qDebug() << rx_buffer;
        unsigned char checkSum0 = byte0(checkSum);
        unsigned char checkSumAdd0 = byte0(checkSumAdd);
        if((unsigned char)rx_buffer.at(frame_copy_len - 2) == checkSum0 && (unsigned char)rx_buffer.at(frame_copy_len - 1) == checkSumAdd0){
            for(int i = 6; i < frame_cnt + 6; i = i + 2) {
                short temp = 0 | (unsigned char)rx_buffer.at(i + 1);
                temp = temp << 8;
                temp = temp | (unsigned char)rx_buffer.at(i);
                info_buffer.append(temp);
            }
            info_buffer.append((unsigned char)rx_buffer.at(3)); // 这一行的目的是因为增加了功能码，在数据末尾增加了功能码用于上层处理
            emit successSignal(info_buffer);
        }
        else qDebug()<< "failed";
        state = 0;
        frame_cnt = 0;
        info_buffer.clear();
    }
    if(state == 7){
        // 校验和填充
        rx_buffer[6 + frame_cnt] = last_byte;
        state = 8;
    }
    if(state == 6){
        // 数据内容填充
        --frame_len;
        rx_buffer[6 + frame_cnt] = last_byte;
        ++frame_cnt;
        if(frame_len <= 0){
            state = 7;
        }
    }
    if(state == 5){
        // 长度填充前半部分
        if(last_byte < 254){
            state = 6;
            rx_buffer[5] = last_byte;
            frame_len = frame_len | (unsigned char)last_byte << 8;
            frame_cnt = 0;
        }
        else state = 0;
    }
    if(state == 4){
        // 长度填充后半部分
        if(last_byte < 254){
            state = 5;
            rx_buffer[4] = last_byte;
            frame_len = 0 | (unsigned char)last_byte;
        }
        else state = 0;
    }
    if(state == 3){
        // 功能码检验
        if(last_byte > 0 && last_byte < 0xF9){
            state = 4;
            rx_buffer[3] = last_byte;
        }
        else state = 0;
    }
    if(state == 2){
        // 目标地址检验
        if(last_byte > 0 && last_byte < 0xF9){
            state = 3;
            rx_buffer[2] = last_byte;
        }
        else state = 0;
    }
    if(state == 1){
        // 源地址检验
        if(last_byte > 0 && last_byte < 0xF9){
            state = 2;
            rx_buffer[1] = last_byte;
        }
        else state = 0;
    }
    if(state == 0){
        // 帧头检验
        if(last_byte == 0xAB){
            rx_buffer[0] = last_byte;
            state = 1;
        }
    }
}
