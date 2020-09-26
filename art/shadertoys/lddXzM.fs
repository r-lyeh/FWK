#define line1 h_ e_ l_ l_ o_ _ s_ h_ a_ d_ e_ r_ t_ o_ y_ crlf
#define line2 t_ h_ i_ s_ _ i_ s_ _ m_ y_ _ f_ o_ n_ t_ crlf
#define line3 h_ o_ p_ e_ _ y_ o_ u_ _ l_ i_ k_ e_ _ i_ t_ crlf
#define line4 f_ e_ e_ l_ _ f_ r_ e_ e_ _ t_ o_ _ u_ s_ e_ crlf
#define line5 _ a_ b_ c_ d_ e_ f_ g_ h_ i_ j_ k_ l_ m_ crlf
#define line6 _ n_ o_ p_ q_ r_ s_ t_ u_ v_ w_ x_ y_ z_

//======Start shared code for state
#define pz_stateYOffset 0.0
#define pz_stateBuf 0
#define pz_stateSample(x) texture(iChannel0,x)
vec2 pz_realBufferResolution;
vec2 pz_originalBufferResolution;
float pz_scale;

void pz_initializeState() {
    pz_realBufferResolution     = iChannelResolution[pz_stateBuf].xy;
    pz_originalBufferResolution = pz_stateSample(.5/pz_realBufferResolution).xy;
}

vec2 pz_nr2vec(float nr) {
    return vec2(mod(nr, pz_originalBufferResolution.x)
                      , pz_stateYOffset+floor(nr / pz_originalBufferResolution.x))+.5;
}

vec4 pz_readState(float nr) {
    return pz_stateSample(pz_nr2vec(nr)/pz_realBufferResolution);
}

float pz_resetCount() {
    return pz_readState(1.).z;
}

vec3 pz_position() {
    return pz_readState(3.).xyz;
}

vec2 pz_initializeState(vec2 fragCoord) {
    pz_initializeState();
    
    vec3 position = pz_position();
    fragCoord -= 0.5*iResolution.xy;
    fragCoord *= pz_scale = position.z;
    fragCoord += (0.5 + position.xy) * iResolution.xy ;
    
    return fragCoord;
}
//======End shared code for state

// line function, used in k, s, v, w, x, y, z
float line(vec2 p, vec2 a, vec2 b)
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}

//These functions are re-used by multiple letters
float _u(vec2 uv,float w,float v) {
    return length(vec2(
                abs(length(vec2(uv.x,
                                max(0.0,-(.4-v)-uv.y) ))-w)
               ,max(0.,uv.y-.4))) +.4;
}
float _i(vec2 uv) {
    return length(vec2(uv.x,max(0.,abs(uv.y)-.4)))+.4;
}
float _j(vec2 uv) {
    uv.x+=.2;
    float t = _u(uv,.25,-.15);
    float x = uv.x>0.?t:length(vec2(uv.x,uv.y+.8))+.4;
    return x;
}
float _l(vec2 uv) {
    uv.y -= .2;
    return length(vec2(uv.x,max(0.,abs(uv.y)-.6)))+.4;
}
float _o(vec2 uv) {
    return abs(length(vec2(uv.x,max(0.,abs(uv.y)-.15)))-.25)+.4;
}

// Here is the alphabet
float aa(vec2 uv) {
    uv = -uv;
    float x = abs(length(vec2(max(0.,abs(uv.x)-.05),uv.y-.2))-.2)+.4;
    x = min(x,length(vec2(uv.x+.25,max(0.,abs(uv.y-.2)-.2)))+.4);
    return min(x,(uv.x<0.?uv.y<0.:atan(uv.x,uv.y+0.15)>2.)?_o(uv):length(vec2(uv.x-.22734,uv.y+.254))+.4);
}
float bb(vec2 uv) {
    float x = _o(uv);
    uv.x += .25;
    return min(x,_l(uv));
}
float cc(vec2 uv) {
    float x = _o(uv);
    uv.y= abs(uv.y);
    return uv.x<0.||atan(uv.x,uv.y-0.15)<1.14?x:length(vec2(uv.x-.22734,uv.y-.254))+.4;
}
float dd(vec2 uv) {
    uv.x *= -1.;
    return bb(uv);
}
float ee(vec2 uv) {
    float x = _o(uv);
    return min(uv.x<0.||uv.y>.05||atan(uv.x,uv.y+0.15)>2.?x:length(vec2(uv.x-.22734,uv.y+.254))+.4,
               length(vec2(max(0.,abs(uv.x)-.25),uv.y-.05))+.4);
}
float ff(vec2 uv) {
    uv.x *= -1.;
    uv.x += .05;
    float x = _j(vec2(uv.x,-uv.y));
    uv.y -= .4;
    x = min(x,length(vec2(max(0.,abs(uv.x-.05)-.25),uv.y))+.4);
    return x;
}
float gg(vec2 uv) {
    float x = _o(uv);
    return min(x,uv.x>0.||uv.y<-.65?_u(uv,0.25,-0.2):length(vec2(uv.x+0.25,uv.y+.65))+.4 );
}
float hh(vec2 uv) {
    uv.y *= -1.;
    float x = _u(uv,.25,.25);
    uv.x += .25;
    uv.y *= -1.;
    return min(x,_l(uv));
}
float ii(vec2 uv) {
    return min(_i(uv),length(vec2(uv.x,uv.y-.7))+.4);
}
float jj(vec2 uv) {
    uv.x += .05;
    return min(_j(uv),length(vec2(uv.x-.05,uv.y-.7))+.4);
}
float kk(vec2 uv) {
    float x = line(uv,vec2(-.25,-.1), vec2(0.25,0.4))+.4;
    x = min(x,line(uv,vec2(-.15,.0), vec2(0.25,-0.4))+.4);
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
    float x =atan(uv.x,uv.y-0.15)<1.14&&uv.y>0.?_o(uv):length(vec2(uv.x-.22734,uv.y-.254))+.4;
    
    //)?_o(uv):length(vec2(uv.x-.22734,uv.y+.254))+.4);
    
    uv.x+=.25;
    return min(x,_i(uv));
}
float ss(vec2 uv) {
    
    if (uv.y <.145 && uv.x>0. || uv.y<-.145)
        uv = -uv;
    
    float x = atan(uv.x-.05,uv.y-0.2)<1.14?
                abs(length(vec2(max(0.,abs(uv.x)-.05),uv.y-.2))-.2)+.4:
                length(vec2(uv.x-.231505,uv.y-.284))+.4;
    return x;
}
float tt(vec2 uv) {
    uv.x *= -1.;
    uv.y -= .4;
    uv.x += .05;
    float x = min(_j(uv),length(vec2(max(0.,abs(uv.x-.05)-.25),uv.y))+.4);
    return x;
}
float uu(vec2 uv) {
    return _u(uv,.25,.25);
}
float vv(vec2 uv) {
    uv.x=abs(uv.x);
    return line(uv,vec2(0.25,0.4), vec2(0.,-0.4))+.4;
}
float ww(vec2 uv) {
    uv.x=abs(uv.x);
    return min(line(uv,vec2(0.3,0.4), vec2(.2,-0.4))+.4,
               line(uv,vec2(0.2,-0.4), vec2(0.,0.1))+.4);
}
float xx(vec2 uv) {
    uv=abs(uv);
    return line(uv,vec2(0.,0.), vec2(.3,0.4))+.4;
}
float yy(vec2 uv) {
    return min(line(uv,vec2(.0,-.2), vec2(-.3,0.4))+.4,
               line(uv,vec2(.3,.4), vec2(-.3,-0.8))+.4);
}
float zz(vec2 uv) {
    float l = line(uv,vec2(0.25,0.4), vec2(-0.25,-0.4))+.4;
    uv.y=abs(uv.y);
    float x = length(vec2(max(0.,abs(uv.x)-.25),uv.y-.4))+.4;
    return min(x,l);
}

// Spare Q :)
float Q(vec2 uv) {
    
    float x = _o(uv);
    uv.y += .3;
    uv.x -= .2;
    return min(x,length(vec2(abs(uv.x+uv.y),max(0.,abs(uv.x-uv.y)-.2)))/sqrt(2.) +.4);
}

//Render char if it's up
#define ch(l) if (nr++==ofs) x=min(x,l(uv));

//Make it a bit easier to type text
#define a_ ch(aa);
#define b_ ch(bb);
#define c_ ch(cc);
#define d_ ch(dd);
#define e_ ch(ee);
#define f_ ch(ff);
#define g_ ch(gg);
#define h_ ch(hh);
#define i_ ch(ii);
#define j_ ch(jj);
#define k_ ch(kk);
#define l_ ch(ll);
#define m_ ch(mm);
#define n_ ch(nn);
#define o_ ch(oo);
#define p_ ch(pp);
#define q_ ch(qq);
#define r_ ch(rr);
#define s_ ch(ss);
#define t_ ch(tt);
#define u_ ch(uu);
#define v_ ch(vv);
#define w_ ch(ww);
#define x_ ch(xx);
#define y_ ch(yy);
#define z_ ch(zz);

//Space
#define _ nr++;

//Next line
#define crlf uv.y += 2.0; nr = 0.;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    fragCoord = pz_initializeState(fragCoord);
    
    float scale = 1.0;//3.5-3.0*sin(iGlobalTime*.2);
	vec2 uv = (fragCoord-0.5*iResolution.xy) / iResolution.x * 22.0 * scale;
    
    float ofs = floor(uv.x)+8.;
    uv.x = mod(uv.x,1.0)-.5;
    
    float x = 100.;
    float nr = 0.;
    uv.y -= 5.;
 
    line1;
    line2;
    line3;
    line4;
    line5;
    line6;
        
	vec3 clr = vec3(0.0);
    
    float px = 17.0/iResolution.x*pz_scale;
    
    clr.r = 0.7-0.7*smoothstep(0.49-px,0.49+px, x); // The body
    clr.g = 0.7-0.7*smoothstep(0.00,px*1.5, abs(x-0.49+px)); // Yellow outline
    clr.b = 0.4-0.4*smoothstep(0.43,0.53,1.0-x); // Background with shadow
    clr.rg += 0.12-0.12*smoothstep(0.00,0.1+px, abs(x-0.49+px)); // Yellow glow
    
    if (iMouse.w>0.1) {
        clr.rgb = vec3(smoothstep(0.49-px,0.49+px, x)); 
    }
    
    fragColor = vec4(clamp(clr,0.0,1.0),1.0);
}
