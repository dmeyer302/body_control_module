void lock(){
  Serial.println("Lock");
}

void unlock(){
  //if not running
  domeStatus = 1;
  Serial.println("Unlock");
}

