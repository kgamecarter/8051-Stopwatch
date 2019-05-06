#include <AT89X52.H>

#define true        1
#define false       0
#define ENABLE      0
#define DISABLE     1

#define byte  unsigned char
#define sbyte signed char
#define uint  unsigned int
#define ulong unsigned long

#define RDOT     0x10
#define LDOT     0x20
#define NONUM      10
#define OVERCHAR 0xFF

void scanStopwatch();
void scan(byte* str);

// BCD to 7�q��ܾ�
code byte ssdTable[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0xFF };
// �ɶ��ܼ�
byte ssw = 0, sss = 0, smm = 0, shh = 0, sCount = 0; 
byte defaultLightness = 100; //�G��

void main()
{
    // ��l�ƭp�ɾ�
    TMOD = 0x00;  //13bit mode
    ET0 = 1;
    TR0 = 1;
    EA = 1;
    while (true)
        scanStopwatch();
}

// ��s�X��
void stopwatchUpdate()
{
    if (ssw != 1) //�ϥκX��
        return;
    if (sCount < 112) // �C13bit�Ҧ� �C112 �@��
        sCount++;
    else
    {
        byte tmp = sss + 1;
        sCount = 0;
        if (tmp < 60)
            sss++;
        else
        {
            sss = 0;
            tmp = smm + 1;
            if (tmp < 60)
                smm++;
            else
            {
                smm = 0;
                tmp = shh + 1;
                if (shh <= 90)
                    shh++;
                else
                    shh = 0;
            }
        }
    }
}

void stopwatch_Button2Press() // �X��}��
{
    if (ssw == 0x02)
        return;
    ssw ^= 1;
}

void stopwatch_Button3Press() // �k�s�}��
{
    if (ssw == 1)
    {
        ssw = 2;
        return;
    }
    if (ssw == 2)
    {
        ssw = 1;
        return;
    }
    sss = 0;
    smm = 0;
    sCount = 0;
}

//�p�ɤ��_
idata byte count = 0;
idata byte b2Count = 0;
idata byte b3Count = 0;
// �C��|�I�s112.5��
void timer0_int() interrupt 1
{
    // ��������1
    if (P3_0 == ENABLE)
    {
        if (b2Count < 225)
            b2Count++;
    }
    else
    {
        if (b2Count && b2Count < 70)  // �u��
            stopwatch_Button2Press();
        b2Count = 0;
    }
    // ��������2
    if (P3_1 == ENABLE)
    {
        if (b3Count < 225)
            b3Count++;
    }
    else
    {
        if (b3Count && b3Count < 70)  // �u��
            stopwatch_Button3Press();
        b3Count = 0;
    }

    // ��s�X��
    stopwatchUpdate();
}

//��ܽX��
void scanStopwatch()
{
    byte str[8], tmp = (uint)sCount * 100 / 112; //�@���
    str[0] = tmp % 10;
    str[1] = tmp / 10;
    str[2] = sss % 10 | RDOT; // ���
    str[3] = sss / 10;
    if (str[3] == 0) // �ˬd���L�Ĺs
        str[3] = NONUM;
    str[4] = smm % 10 | RDOT; // ����
    str[5] = smm / 10;
    if (str[5] == 0) // �ˬd����L�Ĺs
        str[5] = NONUM;
    str[6] = shh % 10 | RDOT; // �ɦ�
    str[7] = shh / 10;
    if (str[7] == 0) // �ˬd�ɦ�L�Ĺs
        str[7] = NONUM;

    defaultLightness = sss % 10 * 10 + tmp / 10; // �ά�]�w�G��
    scan(str);
}

void delayus(uint time)
{
    while (time)
    	time--;
}

void delayms(uint time)
{
    while (time)
    {
        uint n = 120;
        while (n)
            n--;
        time--;
    }
}

// ���y���
void scan(byte* str)
{
    byte i;
    for (i = 0; i < 8; i++)
    {
        if (str[i] == OVERCHAR)
            break;
        if (str[i] == NONUM)
        {
            delayms(2);
            continue;
        }
        P2 = ~(1 << i); //���
        P1 = ssdTable[str[i] & 0x0F]; //���
        P1_7 = str[i] & RDOT ? 0 : 1; //�p���I
        delayus((defaultLightness << 1) + 40); // �G��*2=����
        P2 = 0xFF;
        delayus((100 - defaultLightness) << 1); // �G��*2=����
        //SelectLine = 0;
    }
    P1 = ssdTable[NONUM];
    delayms((8 - i) << 1); // delay��16ms
}