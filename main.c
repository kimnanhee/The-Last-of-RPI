#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#include <wiringPi.h>
#include <softTone.h>

#define piezo 25 // buzzer pin

#define MAXTIMINGS 85
#define DHTPIN 7 // dht pin

int dhtval[5] = {0};
int flag=0; 
// flag�� 0�̸�, �� ������ ���� �ݺ� 
// flag�� 1�̸�, 1~3 ������ �ѹ����� ���� 

int pin_arr[7]={1, 4, 5, 26, 27, 28, 29}; // fnd ���� ��(������� abcdefg) 
int fnd_arr[10][7]={
{1, 1, 1, 1, 1, 1, 0}, //0
{0, 1, 1, 0, 0, 0, 0}, //1
{1, 1, 0, 1, 1, 0, 1}, //2
{1, 1, 1, 1, 0, 0, 1}, //3
{0, 1, 1, 0, 0, 1, 1}, //4
{1, 0, 1, 1, 0, 1, 1}, //5
{1, 0, 1, 1, 1, 1, 1}, //6
{1, 1, 1, 0, 0, 0, 0}, //7
{1, 1, 1, 1, 1, 1, 1}, //8
{1, 1, 1, 1, 0, 1, 1} //9
};
int led_pin[3]={21, 22, 23}; // 3���� led ���� �� 

void init() // �� �ʱ�ȭ 
{
	int i;
	for(i=0; i<7; i++)
	{
		pinMode(pin_arr[i], OUTPUT);
		digitalWrite(pin_arr[i], LOW);
	}
	for(i=0; i<3; i++)
	{
		pinMode(led_pin[i], OUTPUT);
		digitalWrite(led_pin[i], LOW);
	}
	softToneCreate(piezo);
}
int read_dht() // error => return -1
{
    uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j=0, i;

    for(i=0; i<5; i++) dhtval[i] = 0; // �迭 �ʱ�ȭ 

	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, LOW);
	delay(18);
	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(40);
	pinMode(DHTPIN, INPUT);

	for (i = 0; i < MAXTIMINGS; i++)
	{
        counter = 0;
        while ( digitalRead( DHTPIN ) == laststate )
        {
   			counter++;
       		delayMicroseconds(1);
       		if (counter == 255) break;
    	}
       	laststate = digitalRead( DHTPIN );

        if (counter == 255) break;

        if ( (i >= 4) && (i % 2 == 0) )
        {
  			dhtval[j / 8] <<= 1;
    		if (counter > 50) dhtval[j / 8] |= 1;
        	j++;
    	}
	}

	if ((j >= 40) && (dhtval[4] == ( ( dhtval[0] + dhtval[1] + dhtval[2] + dhtval[3] ) & 0xFF) ))
    {
		return 0; // success
    }
	else return -1; // fail
}

void func_1()
{
	init(); // �ʱ�ȭ 
	int i, cnt=0;
	for(i=0; i<3; i++) digitalWrite(led_pin[i], LOW); // LED ���� ���� 

	while(1)
	{
		for(i=0; i<3; i++) // ���������� ���� 
		{
			digitalWrite(led_pin[i], HIGH);
			delay(1000);
		}
		for(i=0; i<3; i++) digitalWrite(led_pin[i], LOW);
		delay(1000);

		cnt++;
		if(cnt>=3 && flag==1) return; // 4���� �������� ���(flag==1) 1������ ���� 
	}
}
void func_2()
{
	init();
	int temp, error_cnt=0, cnt=0;
	float f;

	printf("input temp : "); // �µ� �Է¹ޱ� 
	scanf(" %d", &temp);
	while(1) // read dht sensor
	{
		int num = read_dht();
		if(num == -1)
		{
			error_cnt++;
			printf("error\n");
		}
		else
		{
			f = dhtval[2] * 9. / 5. + 32;
            printf("Humi = %d.%d %% Temp = %d.%d C (%.1f F)\n", dhtval[0], dhtval[1], dhtval[2], dhtval[3], f); // ��� 

			if(dhtval[2]-1<= temp && temp <= dhtval[2]+1) // correct
			{
				printf("input temp, temp correct\n\n");
				softToneWrite(piezo, 391);
				delay(100);
				softToneWrite(piezo, 0);
			}
        }

		if(error_cnt > 10)
        {
            printf("dht senseor error\n");
            printf("please enter the q\n");
        }

		delay(2000);
		cnt++;

		if(cnt>5 && flag==1) return; // 4���� �������� ���(flag==1) 1������ ���� 
	}
}
void func_3()
{
	init(); // �ʱ�ȭ 
	int i, j, num=0, cnt=0;
	for(j=0; j<3; j++) digitalWrite(led_pin[j], LOW); // LED ���� ���� 

	while(1)
	{
		for(i=0; i<7; i++)
			digitalWrite(pin_arr[i], fnd_arr[num][i]); // fnd�� ���� ��� 

		num++;
		cnt++;

		if(num>9) num=0;

		if(cnt%3==0) digitalWrite(led_pin[2], HIGH);
		else if(cnt%3==1) digitalWrite(led_pin[0], HIGH);
		else if(cnt%3==2) digitalWrite(led_pin[1], HIGH);

		delay(800);
		softToneWrite(piezo, 391); // ON buzzer
		delay(200);
		softToneWrite(piezo, 0); // OFF buzzer

		if(cnt%3==0)
		{
			for(j=0; j<3; j++) digitalWrite(led_pin[j], LOW);
			delay(100);
		}
		if(num==9 && flag==1) return; // 4���� �������� ���(flag==1) 1������ ����
	}
}

void func_4()
{
	while(1) // 1, 2, 3�� ������ �ݺ��ϸ� ���� 
	{
		func_1();
		func_2();
		func_3();
	}
}
int main()
{
	wiringPiSetup();

	while(1)
	{
		char num;
		printf("\n\n-------------------------\n");
		printf("1. LED\n");
		printf("2. TEMP\n");
		printf("3. FND\n");
		printf("4. ALL\n");
		printf(">select:");
		scanf(" %c", &num);

		if(!(num=='1' || num=='2' || num=='3' || num=='4')) printf("wrong input\n"); // 1, 2, 3, 4 외에 다른 입력
		else
		{
			int status;
			pid_t pid = fork(); // fork()
			if(pid > 0) // parent
			{
				while(1)
                {
					if(num=='2') delay(5000);
                    char data;
                    printf(">");
					scanf(" %c", &data);
                    if(data == 'q' || data == 'Q') // q�� �Է¹޾��� �� 
                    {
                        kill(pid, SIGINT); // fork�� �ڽ� ���μ������� kill signal ���� 
                        while(waitpid(pid, &status, 0)<0); // �ڽ� ���μ����� ���� �� ���� wait
                        break;
						flag=0;
                    }
                }
			}
			else if(pid == 0) // child
			{
				if(num=='1') func_1();
				else if(num=='2') func_2();
                else if(num=='3') func_3();
				else if(num=='4')
				{
					flag=1; // func_1, 2, 3�� �ѹ����� �����ϱ� ���� ���� ���� 
					func_4();
				}
			}
			else printf("fail to fork\n"); // fork error
		}
	}
	return 0;
}
