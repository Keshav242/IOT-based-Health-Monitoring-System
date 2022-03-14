int filter(int A,int n ){
  if(n==1){
  SUM1 = SUM1 - READINGS1[INDEX1];       // Remove the oldest entry from the sum
  READINGS1[INDEX1] = A;           // Add the newest reading to the window
  SUM1 = SUM1 + A;                 // Add the newest reading to the sum
  INDEX1 = (INDEX1+1) % WINDOW_SIZE1;   // Increment the index, and wrap to 0 if it exceeds the window size
  AVERAGED = SUM1 / WINDOW_SIZE1;      // Divide the sum of the window by the window size for the result
  return AVERAGED;
}
else{
  SUM2 = SUM2 - READINGS2[INDEX2];       // Remove the oldest entry from the sum
  READINGS2[INDEX2] = A;           // Add the newest reading to the window
  SUM2 = SUM2 + A;                 // Add the newest reading to the sum
  INDEX2 = (INDEX2+1) % WINDOW_SIZE2;   // Increment the index, and wrap to 0 if it exceeds the window size
  AVERAGED = SUM2 / WINDOW_SIZE2;      // Divide the sum of the window by the window size for the result
  return AVERAGED;
}
}
