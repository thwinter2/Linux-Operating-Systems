long zfunction(long param){
  param &= 0xfffc01ff;
  param >>= 8;
  param |= 0xff000000;
  return param;
}