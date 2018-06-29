void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if ('\0')Serial.println(-1);
  else Serial.println(1);
    String a = "123";
    Serial.println(a);
    Serial.println(a.substring(0, a.length()-1));
    for (char ii = 0; ii < a.length(); ii++)
      a[ii] += 1;
    Serial.println(a);
}

void loop() {
  // put your main code here, to run repeatedly:

}
