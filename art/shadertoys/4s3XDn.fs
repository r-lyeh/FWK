#define line1 H_ e_ l_ l_ o_ _ BOLD S_ h_ a_ d_ e_ r_ t_ o_ y_ BOLD _ t_ h_ i_ s_ _ i_ s_ _ m_ y_
#define line2 BLUE p_ r_ o_ p_ o_ r_ t_ i_ o_ n_ a_ l_ BLUE _ f_ o_ n_ t_ _dot _ I_ _ h_ o_ p_ e_ _ y_ o_ u_
#define line3 GREEN l_ i_ k_ e_ GREEN _ i_ t_ _comma _ RED f_ e_ e_ l_ RED _ ITAL f_ r_ e_ e_ ITAL _ t_ o_ _ u_ s_ e_ _ i_ t_ _dot
#define line4 a_ b_ c_ d_ e_ f_ g_ h_ i_ j_ k_ l_ m_ n_ o_ p_ q_ r_ s_ t_ u_ v_ w_ x_ y_ z_
#define line5 A_ B_ C_ D_ E_ F_ G_ H_ I_ J_ K_ L_ M_ N_ O_ P_ Q_ R_ S_ T_ U_ V_ W_ X_ Y_ Z_
#define line6 _1 _2 _3 _4 _5 _6 _7 _8 _9 _0 _dot _comma _exclam _question _open1 _close1 _dotdot _dotcomma _equal _add _sub _mul _div _lt _gt _hash _and _or _und _open2 _close2 _open3 _close3

// line function, used in k, v, w, x, y, z, 1, 2, 4, 7 and ,
// rest is drawn using (stretched) circle(g)

// todo: distance fields of s,S, J { and }
// todo before we can show shaders :)
// 

float line(vec2 p, vec2 a, vec2 b)
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

//These functions are re-used by multiple letters
float _u(vec2 uv, float w, float v) {
    return length(vec2(
                abs(length(vec2(uv.x,
                                max(0.0,-(.4-v)-uv.y) ))-w)
               ,max(0.,uv.y-.4)));
}
float _i(vec2 uv) {
    return length(vec2(uv.x,max(0.,abs(uv.y)-.4)));
}
float _j(vec2 uv) {
    uv.x+=.2;
    uv.y+=.55;
    float x = uv.x>0.&&uv.y<0.?
                abs(length(uv)-.25)
               :min(length(uv+vec2(0.,.25)),
                    length(vec2(uv.x-.25,max(0.,abs(uv.y-.475)-.475))));
    return x;
}
float _l(vec2 uv) {
    uv.y -= .2;
    return length(vec2(uv.x,max(0.,abs(uv.y)-.6)));
}
float _o(vec2 uv) {
    return abs(length(vec2(uv.x,max(0.,abs(uv.y)-.15)))-.25);
}

// Here is the alphabet
float aa(vec2 uv) {
    uv = -uv;
    float x = abs(length(vec2(max(0.,abs(uv.x)-.05),uv.y-.2))-.2);
    x = min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y-.2)-.2))));
    return min(x,(uv.x<0.?uv.y<0.:atan(uv.x,uv.y+0.15)>2.)?_o(uv):length(vec2(uv.x-.22734,uv.y+.254)));
}
float bb(vec2 uv) {
    float x = _o(uv);
    uv.x += .25;
    return min(x,_l(uv));
}
float cc(vec2 uv) {
    float x = _o(uv);
    uv.y= abs(uv.y);
    return uv.x<0.||atan(uv.x,uv.y-0.15)<1.14?x:
                    min(length(vec2(uv.x+.25,max(0.0,abs(uv.y)-.15))),//makes df right 
                        length(uv+vec2(-.22734,-.254)));
}
float dd(vec2 uv) {
    uv.x *= -1.;
    return bb(uv);
}
float ee(vec2 uv) {
    float x = _o(uv);
    return min(uv.x<0.||uv.y>.05||atan(uv.x,uv.y+0.15)>2.?x:length(vec2(uv.x-.22734,uv.y+.254)),
               length(vec2(max(0.,abs(uv.x)-.25),uv.y-.05)));
}
float ff(vec2 uv) {
    uv.x *= -1.;
    uv.x += .05;
    float x = _j(vec2(uv.x,-uv.y));
    uv.y -= .4;
    x = min(x,length(vec2(max(0.,abs(uv.x-.05)-.25),uv.y)));
    return x;
}
float gg(vec2 uv) {
    float x = _o(uv);
    return min(x,uv.x>0.||atan(uv.x,uv.y+.6)<-2.?
               _u(uv,0.25,-0.2):
               length(uv+vec2(.23,.7)));
}
float hh(vec2 uv) {
    uv.y *= -1.;
    float x = _u(uv,.25,.25);
    uv.x += .25;
    uv.y *= -1.;
    return min(x,_l(uv));
}
float ii(vec2 uv) {
    return min(_i(uv),length(vec2(uv.x,uv.y-.6)));
}
float jj(vec2 uv) {
    uv.x+=.05;
    return min(_j(uv),length(vec2(uv.x-.05,uv.y-.6)));
}
float kk(vec2 uv) {
    float x = line(uv,vec2(-.25,-.1), vec2(0.25,0.4));
    x = min(x,line(uv,vec2(-.15,.0), vec2(0.25,-0.4)));
    uv.x+=.25;
    return min(x,_l(uv));
}
float ll(vec2 uv) {
    return _l(uv);
}
float mm(vec2 uv) {
    //uv.x *= 1.4;
    uv.y *= -1.;
    uv.x-=.175;
    float x = _u(uv,.175,.175);
    uv.x+=.35;
    x = min(x,_u(uv,.175,.175));
    uv.x+=.175;
    return min(x,_i(uv));
}
float nn(vec2 uv) {
    uv.y *= -1.;
    float x = _u(uv,.25,.25);
    uv.x+=.25;
    return min(x,_i(uv));
}
float oo(vec2 uv) {
    return _o(uv);
}
float pp(vec2 uv) {
    float x = _o(uv);
    uv.x += .25;
    uv.y += .4;
    return min(x,_l(uv));
}
float qq(vec2 uv) {
    uv.x = -uv.x;
    return pp(uv);
}
float rr(vec2 uv) {
    uv.x -= .05;
    float x =atan(uv.x,uv.y-0.15)<1.14&&uv.y>0.?_o(uv):length(vec2(uv.x-.22734,uv.y-.254));
    
    //)?_o(uv):length(vec2(uv.x-.22734,uv.y+.254))+.4);
    
    uv.x+=.25;
    return min(x,_i(uv));
}
float ss(vec2 uv) {
    if (uv.y <.225-uv.x*.5 && uv.x>0. || uv.y<-.225-uv.x*.5)
        uv = -uv;
    float a = abs(length(vec2(max(0.,abs(uv.x)-.05),uv.y-.2))-.2);
    float b = length(vec2(uv.x-.231505,uv.y-.284));
    float x = atan(uv.x-.05,uv.y-0.2)<1.14?a:b;
    return x;
}
float tt(vec2 uv) {
    uv.x *= -1.;
    uv.y -= .4;
    uv.x += .05;
    float x = min(_j(uv),length(vec2(max(0.,abs(uv.x-.05)-.25),uv.y)));
    return x;
}
float uu(vec2 uv) {
    return _u(uv,.25,.25);
}
float vv(vec2 uv) {
    uv.x=abs(uv.x);
    return line(uv,vec2(0.25,0.4), vec2(0.,-0.4));
}
float ww(vec2 uv) {
    uv.x=abs(uv.x);
    return min(line(uv,vec2(0.3,0.4), vec2(.2,-0.4)),
               line(uv,vec2(0.2,-0.4), vec2(0.,0.1)));
}
float xx(vec2 uv) {
    uv=abs(uv);
    return line(uv,vec2(0.,0.), vec2(.3,0.4));
}
float yy(vec2 uv) {
    return min(line(uv,vec2(.0,-.2), vec2(-.3,0.4)),
               line(uv,vec2(.3,.4), vec2(-.3,-0.8)));
}
float zz(vec2 uv) {
    float l = line(uv,vec2(0.25,0.4), vec2(-0.25,-0.4));
    uv.y=abs(uv.y);
    float x = length(vec2(max(0.,abs(uv.x)-.25),uv.y-.4));
    return min(x,l);
}

// Capitals
float AA(vec2 uv) {
    float x = length(vec2(
                abs(length(vec2(uv.x,
                                max(0.0,uv.y-.35) ))-0.25)
               ,min(0.,uv.y+.4)));
    return min(x,length(vec2(max(0.,abs(uv.x)-.25),uv.y-.1) ));
}

float BB(vec2 uv) {
    uv.y -=.1;
    uv.y = abs(uv.y);
    float x = length(vec2(
                abs(length(vec2(max(0.0,uv.x),
                                 uv.y-.25))-0.25)
               ,min(0.,uv.x+.25)));
    return min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y)-.5)) ));
}
float CC(vec2 uv) {
    float x = abs(length(vec2(uv.x,max(0.,abs(uv.y-.1)-.25)))-.25);
    uv.y -= .1;
    uv.y= abs(uv.y);
    return uv.x<0.||atan(uv.x,uv.y-0.25)<1.14?x:
                    min(length(vec2(uv.x+.25,max(0.0,abs(uv.y)-.25))),//makes df right 
                        length(uv+vec2(-.22734,-.354)));
}
float DD(vec2 uv) {
    uv.y -=.1;
    //uv.y = abs(uv.y);
    float x = length(vec2(
                abs(length(vec2(max(0.0,uv.x),
                                max(0.0,abs(uv.y)-.25)))-0.25)
               ,min(0.,uv.x+.25)));
    return min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y)-.5)) ));
}
float EE(vec2 uv) {
    uv.y -=.1;
    uv.y = abs(uv.y);
    float x = min(length(vec2(max(0.,abs(uv.x)-.25),uv.y)),
                  length(vec2(max(0.,abs(uv.x)-.25),uv.y-.5)));
    return min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y)-.5))));
}
float FF(vec2 uv) {
    uv.y -=.1;
    float x = min(length(vec2(max(0.,abs(uv.x)-.25),uv.y)),
                  length(vec2(max(0.,abs(uv.x)-.25),uv.y-.5)));
    return min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y)-.5))));
}
float GG(vec2 uv) {
    float x = abs(length(vec2(uv.x,max(0.,abs(uv.y-.1)-.25)))-.25);
    uv.y -= .1;
    float a = atan(uv.x,max(0.,abs(uv.y)-0.25));
    x = uv.x<0.||a<1.14 || a>3.?x:
                    min(length(vec2(uv.x+.25,max(0.0,abs(uv.y)-.25))),//makes df right 
                        length(uv+vec2(-.22734,-.354)));
    x = min(x,line(uv,vec2(.22734,-.1),vec2(.22734,-.354)));
    return min(x,line(uv,vec2(.22734,-.1),vec2(.05,-.1)));
}
float HH(vec2 uv) {
    uv.y -=.1;
    uv.x = abs(uv.x);
    float x = length(vec2(max(0.,abs(uv.x)-.25),uv.y));
    return min(x,length(vec2(uv.x-.25,max(0.,abs(uv.y)-.5))));
}
float II(vec2 uv) {
    uv.y -= .1;
    float x = length(vec2(uv.x,max(0.,abs(uv.y)-.5)));
    uv.y = abs(uv.y);
    return min(x,length(vec2(max(0.,abs(uv.x)-.1),uv.y-.5)));
}
float JJ(vec2 uv) {
    uv.x += .125;
    float x = length(vec2(
                abs(length(vec2(uv.x,
                                min(0.0,uv.y+.15) ))-0.25)
               ,max(0.,max(-uv.x,uv.y-.6))));
    return min(x,length(vec2(max(0.,abs(uv.x-.125)-.125),uv.y-.6)));
}
float KK(vec2 uv) {
    float x = line(uv,vec2(-.25,-.1), vec2(0.25,0.6));
    x = min(x,line(uv,vec2(-.1, .1), vec2(0.25,-0.4)));
//    uv.x+=.25;
    return min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y-.1)-.5))));
}
float LL(vec2 uv) {
    uv.y -=.1;
    float x = length(vec2(max(0.,abs(uv.x)-.2),uv.y+.5));
    return min(x,length(vec2(uv.x+.2,max(0.,abs(uv.y)-.5))));
}
float MM(vec2 uv) {
    uv.y-=.1;
    float x = min(length(vec2(uv.x-.35,max(0.,abs(uv.y)-.5))),
                  line(uv,vec2(-.35,.5),vec2(.0,-.1)));
    x = min(x,line(uv,vec2(.0,-.1),vec2(.35,.5)));
    return min(x,length(vec2(uv.x+.35,max(0.,abs(uv.y)-.5))));
}
float NN(vec2 uv) {
    uv.y-=.1;
    float x = min(length(vec2(uv.x-.25,max(0.,abs(uv.y)-.5))),
                  line(uv,vec2(-.25,.5),vec2(.25,-.5)));
    return min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y)-.5))));
}
float OO(vec2 uv) {
    return abs(length(vec2(uv.x,max(0.,abs(uv.y-.1)-.25)))-.25);
}
float PP(vec2 uv) {
    float x = length(vec2(
                abs(length(vec2(max(0.0,uv.x),
                                 uv.y-.35))-0.25)
               ,min(0.,uv.x+.25)));
    return min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y-.1)-.5)) ));
}
float QQ(vec2 uv) {
    float x = abs(length(vec2(uv.x,max(0.,abs(uv.y-.1)-.25)))-.25);
    uv.y += .3;
    uv.x -= .2;
    return min(x,length(vec2(abs(uv.x+uv.y),max(0.,abs(uv.x-uv.y)-.2)))/sqrt(2.));
}
float RR(vec2 uv) {
    float x = length(vec2(
                abs(length(vec2(max(0.0,uv.x),
                                 uv.y-.35))-0.25)
               ,min(0.,uv.x+.25)));
    x = min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y-.1)-.5)) ));
    return min(x,line(uv,vec2(0.0,0.1),vec2(0.25,-0.4)));
}
float SS(vec2 uv) {
    uv.y -= .1;
    if (uv.y <.275-uv.x*.5 && uv.x>0. || uv.y<-.275-uv.x*.5)
        uv = -uv;
    float a = abs(length(vec2(max(0.,abs(uv.x)),uv.y-.25))-.25);
    float b = length(vec2(uv.x-.236,uv.y-.332));
    float x = atan(uv.x-.05,uv.y-0.25)<1.14?a:b;
    return x;
}
float TT(vec2 uv) {
    uv.y -= .1;
    float x = length(vec2(uv.x,max(0.,abs(uv.y)-.5)));
    return min(x,length(vec2(max(0.,abs(uv.x)-.25),uv.y-.5)));
}
float UU(vec2 uv) {
    float x = length(vec2(
                abs(length(vec2(uv.x,
                                min(0.0,uv.y+.15) ))-0.25)
               ,max(0.,uv.y-.6)));
    return x;
}
float VV(vec2 uv) {
    uv.x=abs(uv.x);
    return line(uv,vec2(0.25,0.6), vec2(0.,-0.4));
}
float WW(vec2 uv) {
    uv.x=abs(uv.x);
    return min(line(uv,vec2(0.3,0.6), vec2(.2,-0.4)),
               line(uv,vec2(0.2,-0.4), vec2(0.,0.2)));
}
float XX(vec2 uv) {
    uv.y -= .1;
    uv=abs(uv);
    return line(uv,vec2(0.,0.), vec2(.3,0.5));
}
float YY(vec2 uv) {
    return min(min(line(uv,vec2(.0, .1), vec2(-.3, 0.6)),
                   line(uv,vec2(.0, .1), vec2( .3, 0.6))),
                   length(vec2(uv.x,max(0.,abs(uv.y+.15)-.25))));
}
float ZZ(vec2 uv) {
    float l = line(uv,vec2(0.25,0.6), vec2(-0.25,-0.4));
    uv.y-=.1;
    uv.y=abs(uv.y);
    float x = length(vec2(max(0.,abs(uv.x)-.25),uv.y-.5));
    return min(x,l);
}

//Numbers
float _11(vec2 uv) {
    return min(min(
             line(uv,vec2(-0.2,0.45),vec2(0.,0.6)),
             length(vec2(uv.x,max(0.,abs(uv.y-.1)-.5)))),
             length(vec2(max(0.,abs(uv.x)-.2),uv.y+.4)));
             
}
float _22(vec2 uv) {
    float x = min(line(uv,vec2(0.185,0.17),vec2(-.25,-.4)),
                  length(vec2(max(0.,abs(uv.x)-.25),uv.y+.4)));
    uv.y-=.35;
    uv.x += 0.025;
    return min(x,abs(atan(uv.x,uv.y)-0.63)<1.64?abs(length(uv)-.275):
               length(uv+vec2(.23,-.15)));
}
float _33(vec2 uv) {
    uv.y-=.1;
    uv.y = abs(uv.y);
    uv.y-=.25;
    return atan(uv.x,uv.y)>-1.?abs(length(uv)-.25):
           min(length(uv+vec2(.211,-.134)),length(uv+vec2(.0,.25)));
}
float _44(vec2 uv) {
    float x = min(length(vec2(uv.x-.15,max(0.,abs(uv.y-.1)-.5))),
                  line(uv,vec2(0.15,0.6),vec2(-.25,-.1)));
    return min(x,length(vec2(max(0.,abs(uv.x)-.25),uv.y+.1)));
}
float _55(vec2 uv) {
    float b = min(length(vec2(max(0.,abs(uv.x)-.25),uv.y-.6)),
                  length(vec2(uv.x+.25,max(0.,abs(uv.y-.36)-.236))));
    uv.y += 0.1;
    uv.x += 0.05;
    float c = abs(length(vec2(uv.x,max(0.,abs(uv.y)-.0)))-.3);
    return min(b,abs(atan(uv.x,uv.y)+1.57)<.86 && uv.x<0.?
               length(uv+vec2(.2,.224))
               :c);
}
float _66(vec2 uv) {
    uv.y-=.075;
    uv = -uv;
    float b = abs(length(vec2(uv.x,max(0.,abs(uv.y)-.275)))-.25);
    uv.y-=.175;
    float c = abs(length(vec2(uv.x,max(0.,abs(uv.y)-.05)))-.25);
    return min(c,cos(atan(uv.x,uv.y+.45)+0.65)<0.||(uv.x>0.&& uv.y<0.)?b:
               length(uv+vec2(0.2,0.6)));
}
float _77(vec2 uv) {
    return min(length(vec2(max(0.,abs(uv.x)-.25),uv.y-.6)),
               line(uv,vec2(-0.25,-0.39),vec2(0.25,0.6)));
}
float _88(vec2 uv) {
    float l = length(vec2(max(0.,abs(uv.x)-.08),uv.y-.1+uv.x*.07));
    uv.y-=.1;
    uv.y = abs(uv.y);
    uv.y-=.245;
    return min(abs(length(uv)-.255),l);
}
float _99(vec2 uv) {
    uv.y-=.125;
    float b = abs(length(vec2(uv.x,max(0.,abs(uv.y)-.275)))-.25);
    uv.y-=.175;
    float c = abs(length(vec2(uv.x,max(0.,abs(uv.y)-.05)))-.25);
    return min(c,cos(atan(uv.x,uv.y+.45)+0.65)<0.||(uv.x>0.&& uv.y<0.)?b:
               length(uv+vec2(0.2,0.6)));
}
float _00(vec2 uv) {
    uv.y-=.1;
    return abs(length(vec2(uv.x,max(0.,abs(uv.y)-.25)))-.25);
}

//Symbols
float ddot(vec2 uv) {
    uv.y+=.4;
    return length(uv)*0.97;//-.03;
}
float comma(vec2 uv) {
    return min(ddot(uv),line(uv,vec2(.031,-.405),vec2(-.029,-.52)));
}
float exclam(vec2 uv) {
    return min(ddot(uv),length(vec2(uv.x,max(0.,abs(uv.y-.2)-.4)))-uv.y*.06);
}
float question(vec2 uv) {
    float x = min(ddot(uv),length(vec2(uv.x,max(0.,abs(uv.y+.035)-.1125))));
    uv.y-=.35;
    uv.x += 0.025;
    return min(x,abs(atan(uv.x,uv.y)-1.05)<2.?abs(length(uv)-.275):
               length(uv+vec2(.225,-.16))-.0);
}
float open1(vec2 uv) {
    uv.x-=.62;
    return abs(atan(uv.x,uv.y)+1.57)<1.?
            abs(length(uv)-.8)
           :length(vec2(uv.x+.435,abs(uv.y)-.672));
}
float close1(vec2 uv) {
    uv.x = -uv.x;
    return open1(uv);
}
float dotdot(vec2 uv) {
    uv.y -= .1;
    uv.y = abs(uv.y);
    uv.y-=.25;
    return length(uv);
}
float dotcomma(vec2 uv) {
    uv.y -= .1;
    float x = line(uv,vec2(.0,-.28),vec2(-.029,-.32));
    uv.y = abs(uv.y);
    uv.y-=.25;
    return min(length(uv),x);
}
float eequal(vec2 uv) {
    uv.y -= .1;
    uv.y = abs(uv.y);
    return length(vec2(max(0.,abs(uv.x)-.25),uv.y-.15));
}
float aadd(vec2 uv) {
    uv.y -= .1;
    return min(length(vec2(max(0.,abs(uv.x)-.25),uv.y)),
               length(vec2(uv.x,max(0.,abs(uv.y)-.25))));
}
float ssub(vec2 uv) {
    return length(vec2(max(0.,abs(uv.x)-.25),uv.y-.1));
}
float mmul(vec2 uv) {
    uv.y -= .1;
    uv = abs(uv);
    return min(line(uv,vec2(0.866*.25,0.5*.25),vec2(0.))
              ,length(vec2(uv.x,max(0.,abs(uv.y)-.25))));
}
float ddiv(vec2 uv) {
    return line(uv,vec2(-0.25,-0.4),vec2(0.25,0.6));
}
float lt(vec2 uv) {
    uv.y-=.1;
    uv.y = abs(uv.y);
    return line(uv,vec2(0.25,0.25),vec2(-0.25,0.));
}
float gt(vec2 uv) {
    uv.x=-uv.x;
    return lt(uv);
}
float hash(vec2 uv) {
    uv.y-=.1;
    uv.x -= uv.y*.1;
    uv = abs(uv);
    return min(length(vec2(uv.x-.125,max(0.,abs(uv.y)-.3))),
               length(vec2(max(0.,abs(uv.x)-.25),uv.y-.125)));
}
float and(vec2 uv) {
    uv.y-=.44;
    uv.x+=.05;
    float x = abs(atan(uv.x,uv.y))<2.356?abs(length(uv)-.15):1.0;
    x = min(x,line(uv,vec2(-0.106,-0.106),vec2(0.4,-0.712)));
    x = min(x,line(uv,vec2( 0.106,-0.106),vec2(-0.116,-0.397)));
    uv.x-=.025;
    uv.y+=.54;
    x = min(x,abs(atan(uv.x,uv.y)-.785)>1.57?abs(length(uv)-.2):1.0);
    return min(x,line(uv,vec2( 0.141,-0.141),vec2( 0.377,0.177)));
}
float or(vec2 uv) {
    uv.y -= .1;
    return length(vec2(uv.x,max(0.,abs(uv.y)-.5)));
}
float und(vec2 uv) {
    return length(vec2(max(0.,abs(uv.x)-.25),uv.y+.4));
}
float open2(vec2 uv) {
    uv.y -= .1;
    uv.y = abs(uv.y);
    return min(length(vec2(uv.x+.125,max(0.,abs(uv.y)-.5))),
               length(vec2(max(0.,abs(uv.x)-.125),uv.y-.5)));
}
float close2(vec2 uv) {
    uv.x=-uv.x;
    return open2(uv);
}
float open3(vec2 uv) {
    uv.y -= .1;
    uv.y = abs(uv.y);
    float x = length(vec2(
                abs(length(vec2((uv.x*sign(uv.y-.25)-.2),
                            max(0.0,abs(uv.y-.25)-.05) ))-0.2)
               ,max(0.,abs(uv.x)-.2)));
    return  x;
    
}
float close3(vec2 uv) {
    uv.x=-uv.x;
    return open3(uv);
}

vec2 clc(vec2 uv, float cp, float w, float ital) {
    return uv-vec2(cp-(w*.5)+uv.y*ital,0.);
}
bool hit(vec2 uv,inout float cp,float w, float px) {
    return abs((cp+=w)-uv.x)<w+.2;
}

//Render char if it's up
#define ch(l,w) if (hit(uv,cp,w,px)) { x=min(x,l(clc(uv,cp,w,ital))); us=cur;}

//Render char always (no effects anymore)
//#define ch(l,w) x = min(x,l(clc(uv,cp+=w,w,ital)));

//Make it a bit easier to type text
#define a_ ch(aa,0.7);
#define b_ ch(bb,0.7);
#define c_ ch(cc,0.7);
#define d_ ch(dd,0.7);
#define e_ ch(ee,0.7);
#define f_ ch(ff,0.6);
#define g_ ch(gg,0.7);
#define h_ ch(hh,0.7);
#define i_ ch(ii,0.3);
#define j_ ch(jj,0.3);
#define k_ ch(kk,0.7);
#define l_ ch(ll,0.3);
#define m_ ch(mm,0.9);
#define n_ ch(nn,0.7);
#define o_ ch(oo,0.7);
#define p_ ch(pp,0.7);
#define q_ ch(qq,0.7);
#define r_ ch(rr,0.7);
#define s_ ch(ss,0.7);
#define t_ ch(tt,0.7);
#define u_ ch(uu,0.7);
#define v_ ch(vv,0.7);
#define w_ ch(ww,0.9);
#define x_ ch(xx,0.8);
#define y_ ch(yy,0.8);
#define z_ ch(zz,0.7);
#define A_ ch(AA,0.7);
#define B_ ch(BB,0.7);
#define C_ ch(CC,0.7);
#define D_ ch(DD,0.7);
#define E_ ch(EE,0.7);
#define F_ ch(FF,0.7);
#define G_ ch(GG,0.7);
#define H_ ch(HH,0.7);
#define I_ ch(II,0.5);
#define J_ ch(JJ,0.5);
#define K_ ch(KK,0.7);
#define L_ ch(LL,0.5);
#define M_ ch(MM,0.9);
#define N_ ch(NN,0.7);
#define O_ ch(OO,0.7);
#define P_ ch(PP,0.7);
#define Q_ ch(QQ,0.7);
#define R_ ch(RR,0.7);
#define S_ ch(SS,0.7);
#define T_ ch(TT,0.7);
#define U_ ch(UU,0.7);
#define V_ ch(VV,0.7);
#define W_ ch(WW,0.9);
#define X_ ch(XX,0.8);
#define Y_ ch(YY,0.8);
#define Z_ ch(ZZ,0.7);
#define _1 ch(_11,0.7);
#define _2 ch(_22,0.7);
#define _3 ch(_33,0.7);
#define _4 ch(_44,0.7);
#define _5 ch(_55,0.7);
#define _6 ch(_66,0.7);
#define _7 ch(_77,0.7);
#define _8 ch(_88,0.7);
#define _9 ch(_99,0.7);
#define _0 ch(_00,0.7);
#define _dot ch(ddot,0.3);
#define _comma ch(comma,0.3);
#define _exclam ch(exclam,0.3);
#define _question ch(question,0.8);
#define _open1 ch(open1,0.7);
#define _close1 ch(close1,0.7);
#define _dotdot ch(dotdot,0.3);
#define _dotcomma ch(dotcomma,0.3);
#define _equal ch(eequal,0.7);
#define _add ch(aadd,0.7);
#define _sub ch(ssub,0.7);
#define _mul ch(mmul,0.7);
#define _div ch(ddiv,0.7);
#define _lt ch(lt,0.7);
#define _gt ch(gt,0.7);
#define _hash ch(hash,0.7);
#define _and ch(and,0.9);
#define _or ch(or,0.3);
#define _und ch(und,0.7);
#define _open2 ch(open2,0.6);
#define _close2 ch(close2,0.6);
#define _open3 ch(open3,0.7);
#define _close3 ch(close3,0.7);
    
//Space
#define _ cp+=.5;

//Markup
#define BOLD cur.w = 1.5-cur.w;
#define ITAL ital = 0.15-ital;
#define RED cur.r = 0.8-cur.r;
#define GREEN cur.g = 0.6-cur.g;
#define BLUE cur.b = 1.0-cur.b;

//Next line
#define crlf uv.y += 2.0; cp = 0.;


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float ms = float(iMouse.w>0.);
    float scale = 1.1-ms;// 2.0-cos(iGlobalTime*.1);
    vec2 mouseOffs = ms*(iMouse.xy-.5*iResolution.xy)/scale;
	vec2 uv = (fragCoord+mouseOffs-.5*iResolution.xy) / iResolution.x * 22.0 * scale;
    
    //float ofs = floor(uv.x)+8.;
    //uv.x = mod(uv.x,1.0)-.5;
    
    float px = 22.0/iResolution.x*scale;
    float x = 100.;
    float cp = 0.;
    vec4 cur = vec4(0.,0.,0.,0.5);
    vec4 us = cur;
    float ital = 0.0;
    //uv+= .03*cos(uv*7.+iGlobalTime);
    //uv.y += 2.;
    uv.x += 10.1;
    //uv.y -= 3.;
    //uv.x += 5.;
    
    int lnr = 2-int(floor(uv.y/2.));
    uv.y = mod(uv.y,2.0)-1.0;
    
    if (lnr==0) {line1}
    if (lnr==1) {line2}
    if (lnr==2) {line3}
    if (lnr==3) {line4}
    if (lnr==4) {line5}
    if (lnr==5) {line6}
        
	vec3 clr = vec3(0.0);
   
    // Wobbly font
    //float weight = 0.05+sin(length(-10.0+uv*3.9-10.0*sin(.09*iGlobalTime)))/50.;
    
    float weight = 0.01+us.w*min(iGlobalTime*.02-.05,0.03);//+.03*length(sin(uv*6.+.3*iGlobalTime));//+0.02-0.06*cos(iGlobalTime*.4+1.);
    if (ms>0.) {
        fragColor = vec4(mix(us.rgb+0.5-0.5*smoothstep(0.,2.*px, abs(mod(x,.1)-.05)),vec3(1.0), sqrt(x)),1.0);
    } else
        fragColor = vec4(mix(us.rgb,vec3(1.0),smoothstep(weight-px,weight+px, x)),1.0);
}
