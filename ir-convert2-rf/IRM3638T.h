//功能 ：测验红外遥控的收，发功能. 
#ifndef __IRM3638T__
#define __IRM3638T__

//初始化CCP0  使用CCP0管脚，采样脉冲信号，利用捕获功能，计算高电平宽度和低电平宽度
void Init_CCP0(void);

//发射一帧完整的NEC信息 引导码+用户码低八位+用户码高八位+8位数据码+8位数据码的反码+结束码“0”
void Send_NEC_Message(unsigned int Code_User,unsigned char Code_Data);

//解码CCP中断接收的数据。这个函数需要放到主函数里面循环调用才可以。
//本函数返回0表示接收到了有效数据，返回1表示没有接收到有效帧，返回2表示解码错误
unsigned char Get_NEC_Message(void);

//发射射频码，模仿红外信号
void SendRfCode(unsigned int Code_User,unsigned char Code_Data);
#endif
