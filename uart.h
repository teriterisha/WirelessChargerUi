#ifndef UART_H
#define UART_H

#include <QByteArray>
#include <QList>

#include <QObject>
class UartTranC
{
public:
    UartTranC(int tx_buffer_len = 100);
    void uart_frame_tx(unsigned char source_code, unsigned char target_code, unsigned char fun_code,int data_len, const QList<short> &data_tx);

    int reserve_len; // 保留长度
    int tx_frame_cnt;
    QByteArray tx_buffer;
};

class UartRecC : public QObject
{
    Q_OBJECT
public:
    explicit UartRecC(QObject *parent = nullptr,int state = 0, int frame_cnt = 0, int frame_len = 0, int frame_copy_len = 50, int bufflen=500, unsigned char info_header = 0xF1);

    void uart_frame_seg();

    int state;
    int frame_cnt;
    int frame_len;
    int frame_copy_len;
    unsigned char info_header;
    unsigned char last_byte;

    QByteArray rx_buffer;
    QList<short> info_buffer;
signals:
    void successSignal(QList<short>);
};
#endif // UART_H
