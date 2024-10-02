float  sigmoid(float t,float k, float b) { // 0<=t<=1; 0<b<1; -1<=k<=1
  // see https://www.desmos.com/calculator/ym0phzdvll
  float k1=k-1;
  float a = b*k1/(4*b*k-k1);
  float top = b*k1*(t/b-1)/(k*4*(t-b)-k1) + a;
  float bot = b*k1*(1/b-1)/(k*4*(1-b)-k1) + a;
  return top/bot;
}
void setup(){}
void loop(){}