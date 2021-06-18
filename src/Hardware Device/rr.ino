void get_rr(){
  
  double data = (double)analogRead(pin);
  peakDetection.add(data);
  int peak = peakDetection.getPeak();
    double filtered = peakDetection.getFilt();
    if((previous_peak*peak == -1 )&& peak ==-1){
    flag_mila = 1;
    inhale_time = w*exhale_time+ (1-w)*(millis()-t)/1000;
    t = millis();
  }
  else if((previous_peak*peak == -1)&& peak ==1){
    flag_mila = 1;
    exhale_time = w*exhale_time+ (1-w)*(millis()-t)/1000;
    t = millis();
  }
  else 
    flag_mila = 0;
  breadth_time = inhale_time +exhale_time;
  Serial.print(data);
  Serial.print(",");
  Serial.print(100*peak+600);
  Serial.print(",");
  Serial.print(100*flag_mila+600);
  Serial.print(",");
  Serial.print(filtered);
  Serial.print(",");
  Serial.print(inhale_time+600);
  Serial.print(",");
  Serial.print(exhale_time+600);
  Serial.print(",");
  Serial.println(breadth_time);

  if(abs(peak)>0)
    previous_peak = peak;
 // delay(10);
  
}
