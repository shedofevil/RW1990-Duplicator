/* Облегчённая версия дубликатора RW1990 (Бета)
 *  
 *  Приницип работы:
 *  1) Прикладываем оригинал ключа или посылаем в терминал 'w'. Если второе, то вводим ключ вручную.
 *  2) После появления ID ключа в терминале (если проверка не пройдена, происходит возврат), в течении 5 секунд меняем его на пустую заготовку (мигает светодиод).
 *  3) Происходит запись (каждый байт ключа отображается в терминале как "**"
 *  4) После того, как пробежали звёздочки, не убираем ключ и ждём сверки ключа из заготовки с оригинальным ключом.
 *  5) Результат сравнения ключей отобразится в терминале. В течение 5 секунд уберите заготовку (мигает светодиод).
 *  
 */

#include <OneWire.h>

OneWire ibutton (8); // назначаем контакт считывателя

int mnulvl = 0; // переменная для уровня меню

byte key[8]; // массив для сохранения ключа
byte newkey[8]; // массив для сохранения повторно считанного ключа

char inkey[8]; // массив для ввода ключа (буфер)

void setup() {

pinMode(13, OUTPUT); // инициализируем светодиод
Serial.begin(115200); // стартуем монитор порта для отладки и управления

showMenu();

}

void loop() {

switch (mnulvl) {
  
case 0: { // ищем ключ и слушаем терминал

  if (!ibutton.search (key)){
  ibutton.reset_search();
  delay(200);
    if (Serial.read()=='w'){
    mnulvl = 1; // если данные пришли из терминала, переходим к обработке
    return;
    }
  return; }
  mnulvl = 2; // если таблетка найдена, переходим к записи
  
break;
}

case 1: { // обрабатываем введённый в терминале ключ

Serial.println("Please enter the key in 01...CRC format:");

  while(Serial.available() == 0){} // собираем байты в буфер
  delay(100);
  int i=0;
  while(Serial.available()) {
  inkey[i] = Serial.read(); i++;
  }
  inkey[i] = '\0'; // конец буфера

  Serial.print("You are typed this: ");
  Serial.println(inkey); // для визуальной проверки правильности ввода

  sscanf(inkey,"%2x%2x%2x%2x%2x%2x%2x%2x",&key[0], &key[1],&key[2],&key[3],&key[4],&key[5],&key[6],&key[7]); // по шаблону собираем символы в HEX массив 

  // сброс буфера
  for (byte x = 0; x<8; x++){
  inkey[x]=0;
  }

  mnulvl = 2; // переходим к записи
  
break;
}

case 2: { // записываем ключ

  Serial.println("And here the key...");
  
// побайтовый вывод ключа в терминал
  for (int x = 7; x>=0; x--){
    
  // подставляем ведущий нуль
  if (key[x]<16) {
  Serial.print("0");}
  
  Serial.print(key[x],HEX);
  Serial.print(" "); }

  Serial.println();

// вычисляем CRC8

  byte crc;
  crc = ibutton.crc8(key, 7);
  Serial.print("CRC: ");
  Serial.println(crc,HEX);

  if (crc == key[7] && crc > 0 && key[0] == 1) {
    Serial.println("Key looks OK!");
  }

  else
{
  Serial.println("Key looks wrong!");
      for (byte x = 0; x<8; x++){
      key[x]=0;
    }

      for (byte x = 0; x<20; x++) { // миагем светодиодом, предупреждая о возврате в начало
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
    
    mnulvl = 0; // всё плохо, возвращаемся в начало

  ledAlert();
  showMenu(); // вызываем меню
  
  return;
}

  Serial.println("Detach the key in 5 sec...");
  
  for (int x = 5; x>0; x--){  // неистово мигаем светодиодом и ведём обратный отсчёт
  
  digitalWrite(13, HIGH);
  delay (500);
  digitalWrite(13, LOW);
  delay (500);
    
  Serial.println(x);
  }
  
  Serial.println();
  Serial.println("Writing the key...");
  
// запись

    ibutton.skip();
    ibutton.reset();
    ibutton.write(0xD5, 1);
    for (byte x = 0; x<8; x++){
      writeByte(key[x]);
      Serial.print("**");
    }
    Serial.println();
    ibutton.reset();

  Serial.println();

  ibutton.reset_search(); // повторно считываем ключ и сверяем считанный в начале ключ со свежезаписанным ключом
  ibutton.search (newkey);

  if (key[0]==newkey[0] && key[1]==newkey[1] && key[2]==newkey[2] && key[3]==newkey[3] && key[4]==newkey[4] && key[5]==newkey[5] && key[6]==newkey[6] && key[7]==newkey[7]){
  Serial.println("   Key is OK!   ");}

  else
{
  Serial.println("Something wrong!");
}

  // сброс переменной повторно считанного ключа (буфера)
    for (byte x = 0; x<8; x++){
      newkey[x]=0;
    }

  ledAlert(); 
  showMenu(); // вызываем меню

mnulvl = 0; // возвращаемся в начало

break;
}
}
  
} // конец void loop

int writeByte(byte data){
  int data_bit;
  for(data_bit=0; data_bit<8; data_bit++){
    if (data & 1){
      digitalWrite(8, LOW);
      delayMicroseconds(60);
      digitalWrite(8, HIGH);
      delay(10);
    } else {
      digitalWrite(8, LOW);
      digitalWrite(8, HIGH);
      delay(10);
    }
    data = data >> 1;
  }
  return 0;
}

int showMenu() {
  Serial.println("-------------------------------");
  Serial.println();
  Serial.println("Searching for the key..."); 
  Serial.println("Type 'w' if you wish to input the key manually.");
        return 0;
} 

int ledAlert() {
      for (byte x = 0; x<25; x++) { // миагем светодиодом, предупреждая о возврате в начало
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
      delay(100);
    }
}




