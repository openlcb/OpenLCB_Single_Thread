// See: https://www.desmos.com/calculator/ym0phzdvll
 // \frac{\left(\frac{b\left(k-1\right)\left(\frac{x}{b}-1\right)}{\left(4k\left|x-b\right|-k-1\right)}+\frac{b\left(k-1\right)}{4bk-k-1}\right)}{\left(\frac{b\left(k-1\right)\left(\frac{1}{b}-1\right)}{\left(4k\left|1-b\right|-k-1\right)}+\frac{b\left(k-1\right)}{4bk-k-1}\right)}
// returns sigmoid, k is degree of sigmoid, b is inflection point.
// k = -1 to 1, 0=linear
// b = 0.005 to 1, inflection is at b
float sigmoid(float x, float k, float b) {
  float k1 = k-1;
  float term = b*k1/(4*b*k-k-1);
  float top = (b*k1*(x/b-1))/(4*k*abs(x-b)-k-1) + term;
  float bottom = b*k1*(1/b-1)/(4*k*abs(1-b)-k-1) + term;
  return top/bottom;
}

/*
            <int>
              <name>Ease</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>Linear</value></relation>
                <relation><property>1</property><value>Minimum</value></relation>
                <relation><property>2</property><value>Medium</value></relation>
                <relation><property>3</property><value>Maximum</value></relation>
              </map>
            </int>
            <int>
              <name>Asymmetry</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>None</value></relation>
                <relation><property>1</property><value>Max end</value></relation>
                <relation><property>2</property><value>Medium end</value></relation>
                <relation><property>3</property><value>Medium beginning</value></relation>
                <relation><property>4</property><value>Max beginning</value></relation>
              </map>
            </int>
*/

/*
            uint8_t ease;
            uint8_t asym;
*/


