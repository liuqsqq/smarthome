//���� ���������ң�ص��գ�������. 
#ifndef __IRM3638T__
#define __IRM3638T__

//��ʼ��CCP0  ʹ��CCP0�ܽţ����������źţ����ò����ܣ�����ߵ�ƽ��Ⱥ͵͵�ƽ���
void Init_CCP0(void);

//����һ֡������NEC��Ϣ ������+�û���Ͱ�λ+�û���߰�λ+8λ������+8λ������ķ���+�����롰0��
void Send_NEC_Message(unsigned int Code_User,unsigned char Code_Data);

//����CCP�жϽ��յ����ݡ����������Ҫ�ŵ�����������ѭ�����òſ��ԡ�
//����������0��ʾ���յ�����Ч���ݣ�����1��ʾû�н��յ���Ч֡������2��ʾ�������
unsigned char Get_NEC_Message(void);

//������Ƶ�룬ģ�º����ź�
void SendRfCode(unsigned int Code_User,unsigned char Code_Data);
#endif
