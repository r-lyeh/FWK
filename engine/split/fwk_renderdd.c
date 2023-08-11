static const char *dd_vs = "//" FILELINE "\n"
    "in vec3 att_position;\n"
    "uniform mat4 u_MVP;\n"
    "uniform vec3 u_color;\n"
    "out vec3 out_color;\n"
    "void main() {\n"
    "    gl_Position = u_MVP * vec4(att_position, 1.0);\n"
    "    gl_PointSize = 4.0; /* for GL_POINTS draw commands */\n"
    "    out_color = u_color;\n"
    "}";

static const char *dd_fs = "//" FILELINE "\n"
    // "precision mediump float;\n"
    "in vec3 out_color;\n"
    "out vec4 fragcolor;\n"
    "void main() {\n"
    "   fragcolor = vec4(out_color, 1.0);\n"
    "}";

#define X(x) RGBX(x,255)
const uint32_t secret_palette[32] = { // pico8 secret palette (CC0, public domain)
X(0x000000),X(0x1D2B53),X(0x7E2553),X(0x008751),X(0xAB5236),X(0x5F574F),X(0xC2C3C7),X(0xFFF1E8), /*00.07*/
X(0xFF004D),X(0xFFA300),X(0xFFEC27),X(0x00E436),X(0x29ADFF),X(0x83769C),X(0xFF77A8),X(0xFFCCAA), /*08.15*/
X(0x291814),X(0x111D35),X(0x422136),X(0x125359),X(0x742F29),X(0x49333B),X(0xA28879),X(0xF3EF7D), /*16.23*/
X(0xBE1250),X(0xFF6C24),X(0xA8E72E),X(0x00B543),X(0x065AB5),X(0x754665),X(0xFF6E59),X(0xFF9D81), /*24.31*/
};
#undef X

typedef struct text2d_cmd {
    const char *str;
    uint32_t col;
    vec3 pos;
    float sca;
} text2d_cmd;

static uint32_t                  dd_color = ~0u;
static GLuint                    dd_program = -1;
static int                       dd_u_color = -1;
static map(unsigned,array(vec3)) dd_lists[2][3] = {0}; // [0/1 ontop][0/1/2 thin lines/thick lines/points]
static bool                      dd_use_line = 0;
static bool                      dd_ontop = 0;
static array(text2d_cmd)         dd_text2d;
static array(vec4)               dd_matrix2d;

void ddraw_push_2d() {
    float width = window_width();
    float height = window_height();
    float zdepth_max = window_height();
    array_push(dd_matrix2d, vec4(width,height,zdepth_max,0));

    ddraw_flush();
}

void ddraw_pop_2d() {
    vec4 dim = *array_back(dd_matrix2d);
    array_pop(dd_matrix2d);

    mat44 id, proj;
    id44(id);
    ortho44(proj, 0,dim.x,dim.y,0, -dim.z, +dim.z);
    ddraw_flush_projview(proj, id);
}

void ddraw_flush() {
    ddraw_flush_projview(camera_get_active()->proj, camera_get_active()->view);
}

void ddraw_flush_projview(mat44 proj, mat44 view) {
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    mat44 mvp;
    multiply44x2(mvp, proj, view); // MVP where M=id

    glUseProgram(dd_program);
    glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);

    static GLuint vao, vbo;
    if(!vao) glGenVertexArrays(1, &vao);    glBindVertexArray(vao);
    if(!vbo) glGenBuffers(1, &vbo);         glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_PROGRAM_POINT_SIZE); // for GL_POINTS
    glEnable(GL_LINE_SMOOTH); // for GL_LINES (thin)

    for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
        GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
        glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
        for each_map(dd_lists[dd_ontop][i], unsigned, rgb, array(vec3), list) {
            int count = array_count(list);
            if(!count) continue;
                // color
                vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                // config vertex data
                glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                // feed vertex data
                glDrawArrays(mode, 0, count);
                profile_incstat("Render.num_drawcalls", +1);
                profile_incstat(i < 2 ? "Render.num_lines" : "Render.num_points", count);
            array_clear(list);
        }
    }

    if(array_count(dd_text2d)) { // text 2d
        // queue
        for(int i = 0; i < array_count(dd_text2d); ++i) {
            ddraw_color(dd_text2d[i].col);
            ddraw_text(dd_text2d[i].pos, dd_text2d[i].sca, dd_text2d[i].str); 
        }

        // flush
        float mvp[16]; float zdepth_max = 1;
        ortho44(mvp, -window_width()/2, window_width()/2, -window_height()/2, window_height()/2, -1, 1);
        translate44(mvp, -window_width()/2, window_height()/2, 0);
        glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);
        for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
            GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
            glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
            for each_map(dd_lists[dd_ontop][i], unsigned, rgb, array(vec3), list) {
                int count = array_count(list);
                if(!count) continue;
                    // color
                    vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                    glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                    // config vertex data
                    glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                    // feed vertex data
                    glDrawArrays(mode, 0, count);
                    profile_incstat("Render.num_drawcalls", +1);
                    profile_incstat(i < 2 ? "Render.num_lines" : "Render.num_points", count);
                array_clear(list);
            }
        }

        // clear
        array_resize(dd_text2d, 0);
    }
    
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(0);

    ddraw_color(WHITE); // reset color for next drawcall
}

static array(bool) dd_ontops;
void ddraw_ontop(int enabled) {
    dd_ontop = !!enabled;
}
void ddraw_ontop_push(int enabled) {
    array_push(dd_ontops, dd_ontop);
    dd_ontop = !!enabled;
}
void ddraw_ontop_pop() {
    bool *pop = array_pop(dd_ontops);
    if(pop) dd_ontop = *pop;
}

static array(uint32_t) dd_colors;
void ddraw_color(unsigned rgb) {
    dd_color = rgb;
}
void ddraw_color_push(unsigned rgb) {
    array_push(dd_colors, dd_color);
    dd_color = rgb;
}
void ddraw_color_pop() {
    unsigned *pop = array_pop(dd_colors);
    if(pop) dd_color = *pop;
}

void ddraw_point(vec3 from) {
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][2], dd_color, 0);
    array_push(*found, from);
}
void ddraw_line_thin(vec3 from, vec3 to) { // thin lines
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][0], dd_color, 0);
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line(vec3 from, vec3 to) { // thick lines
    array(vec3) *found = map_find_or_add(dd_lists[dd_ontop][1], dd_color, 0);
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line_dashed(vec3 from, vec3 to) { // thick lines
    vec3 dist = sub3(to, from); vec3 unit = norm3(dist);
    for( float len = 0, mag = len3(dist) / 2; len < mag; ++len ) {
        to = add3(from, unit);
        ddraw_line(from, to);
        from = add3(to, unit);
    }
}
void ddraw_triangle(vec3 pa, vec3 pb, vec3 pc) {
    ddraw_line(pa, pb);
    ddraw_line(pa, pc);
    ddraw_line(pb, pc);
}
void ddraw_axis(float units) {
    ddraw_color(RED);   ddraw_line(vec3(0,0,0), vec3(units,0,0)); ddraw_line_dashed(vec3(0,0,0), vec3(-units,0,0));
    ddraw_color(GREEN); ddraw_line(vec3(0,0,0), vec3(0,units,0)); ddraw_line_dashed(vec3(0,0,0), vec3(0,-units,0));
    ddraw_color(BLUE);  ddraw_line(vec3(0,0,0), vec3(0,0,units)); ddraw_line_dashed(vec3(0,0,0), vec3(0,0,-units));
}
void ddraw_ground_(float scale) { // 10x10
    ddraw_color( WHITE ); // outer
    for( float i = -scale, c = 0; c <= 20; c += 20, i += c * (scale/10) ) {
        ddraw_line(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
    ddraw_color( GRAY ); // inner
    for( float i = -scale + scale/10, c = 1; c < 20; ++c, i += (scale/10) ) {
        ddraw_line_thin(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line_thin(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
}
void ddraw_ground(float scale) {
    if( scale ) {
        ddraw_ground_(scale);
    } else {
        ddraw_ground_(100);
        ddraw_ground_(10);
        ddraw_ground_(1);
        ddraw_ground_(0.1);
        ddraw_ground_(0.01);
    }
}
void ddraw_grid(float scale) {
    ddraw_ground(scale);
    ddraw_axis(scale ? scale : 100);
}

void ddraw_text2d(vec2 pos, const char *text) {
    struct text2d_cmd t = {0};
    t.sca = 0.5f; // 0.5 is like vertical 12units each
    t.pos = vec3(pos.x, 0 - pos.y - 12, 0);
    t.str = text;
    t.col = YELLOW;
    array_push(dd_text2d, t);
}

void (ddraw_text)(vec3 pos, float scale, const char *text) {
    // [ref] http://paulbourke.net/dataformats/hershey/ (PD)
    // [ref] https://sol.gfxile.net/hershey/fontprev.html (WTFPL2)
    static const char *hershey[] = { /* simplex font */
    "AQ","IKFVFH@@FCEBFAGBFC","FQEVEO@@MVMO","LVLZE:@@RZK:@@EMSM@@DGRG","[UIZI=@@MZ"
    "M=@@RSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","`YVVDA@@IVKTKRJPHOFODQDSEUGVIVK"
    "UNTQTTUVV@@RHPGOEOCQASAUBVDVFTHRH","c[XMXNWOVOUNTLRGPDNBLAHAFBECDEDGEIFJMNNOOQ"
    "OSNULVJUISIQJNLKQDSBUAWAXBXC","HKFTEUFVGUGSFQEP","KOLZJXHUFQELEHFCH?J<L:","KOD"
    "ZFXHUJQKLKHJCH?F<D:","IQIVIJ@@DSNM@@NSDM","F[NSNA@@EJWJ","IKGBFAEBFCGBG@F>E=",\
    "C[EJWJ","FKFCEBFAGBFC","CWUZC:","RUJVGUERDMDJEEGBJALAOBQERJRMQROULVJV","EUGRIS"
    "LVLA","OUEQERFTGUIVMVOUPTQRQPPNNKDARA","PUFVQVKNNNPMQLRIRGQDOBLAIAFBECDE","GUN"
    "VDHSH@@NVNA","RUPVFVEMFNIOLOONQLRIRGQDOBLAIAFBECDE","XUQSPUMVKVHUFREMEHFDHBKAL"
    "AOBQDRGRHQKOMLNKNHMFKEH","FURVHA@@DVRV","^UIVFUESEQFOHNLMOLQJRHREQCPBMAIAFBECD"
    "EDHEJGLJMNNPOQQQSPUMVIV","XUQOPLNJKIJIGJELDODPESGUJVKVNUPSQOQJPENBKAIAFBED","L"
    "KFOENFMGNFO@@FCEBFAGBFC","OKFOENFMGNFO@@GBFAEBFCGBG@F>E=","DYUSEJUA","F[EMWM@@"
    "EGWG","DYESUJEA","USDQDRETFUHVLVNUOTPRPPONNMJKJH@@JCIBJAKBJC","x\\SNRPPQMQKPJO"
    "ILIIJGLFOFQGRI@@MQKOJLJIKGLF@@SQRIRGTFVFXHYKYMXPWRUTSUPVMVJUHTFREPDMDJEGFEHCJB"
    "MAPASBUCVD@@TQSISGTF","ISJVBA@@JVRA@@EHOH","XVEVEA@@EVNVQURTSRSPRNQMNL@@ELNLQK"
    "RJSHSERCQBNAEA","SVSQRSPUNVJVHUFSEQDNDIEFFDHBJANAPBRDSF","PVEVEA@@EVLVOUQSRQSN"
    "SIRFQDOBLAEA","LTEVEA@@EVRV@@ELML@@EARA","ISEVEA@@EVRV@@ELML","WVSQRSPUNVJVHUF"
    "SEQDNDIEFFDHBJANAPBRDSFSI@@NISI","IWEVEA@@SVSA@@ELSL","CIEVEA","KQMVMFLCKBIAGA"
    "EBDCCFCH","IVEVEA@@SVEH@@JMSA","FREVEA@@EAQA","LYEVEA@@EVMA@@UVMA@@UVUA","IWEV"
    "EA@@EVSA@@SVSA","VWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV","NVEVEA@@EVNVQU"
    "RTSRSORMQLNKEK","YWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV@@MES?","QVEVEA@@"
    "EVNVQURTSRSPRNQMNLEL@@LLSA","UURSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","FQIV"
    "IA@@BVPV","KWEVEGFDHBKAMAPBRDSGSV","FSBVJA@@RVJA","LYCVHA@@MVHA@@MVRA@@WVRA",""
    "FUDVRA@@RVDA","GSBVJLJA@@RVJL","IURVDA@@DVRV@@DARA","LOEZE:@@FZF:@@EZLZ@@E:L:",
    "COAVO>","LOJZJ:@@KZK:@@DZKZ@@D:K:","KQGPISKP@@DMIRNM@@IRIA","CQA?Q?","HKGVFUES"
    "EQFPGQFR","RTPOPA@@PLNNLOIOGNELDIDGEDGBIALANBPD","RTEVEA@@ELGNIOLONNPLQIQGPDNB"
    "LAIAGBED","OSPLNNLOIOGNELDIDGEDGBIALANBPD","RTPVPA@@PLNNLOIOGNELDIDGEDGBIALANB"
    "PD","RSDIPIPKOMNNLOIOGNELDIDGEDGBIALANBPD","IMKVIVGUFRFA@@COJO","WTPOP?O<N;L:I"
    ":G;@@PLNNLOIOGNELDIDGEDGBIALANBPD","KTEVEA@@EKHNJOMOONPKPA","IIDVEUFVEWDV@@EOE"
    "A","LKFVGUHVGWFV@@GOG>F;D:B:","IREVEA@@OOEE@@IIPA","CIEVEA","S_EOEA@@EKHNJOMOO"
    "NPKPA@@PKSNUOXOZN[K[A","KTEOEA@@EKHNJOMOONPKPA","RTIOGNELDIDGEDGBIALANBPDQGQIP"
    "LNNLOIO","RTEOE:@@ELGNIOLONNPLQIQGPDNBLAIAGBED","RTPOP:@@PLNNLOIOGNELDIDGEDGBI"
    "ALANBPD","INEOEA@@EIFLHNJOMO","RROLNNKOHOENDLEJGILHNGOEODNBKAHAEBDD","IMFVFEGB"
    "IAKA@@COJO","KTEOEEFBHAKAMBPE@@POPA","FQCOIA@@OOIA","LWDOHA@@LOHA@@LOPA@@TOPA",
    "FRDOOA@@OODA","JQCOIA@@OOIAG=E;C:B:","IROODA@@DOOO@@DAOA","hOJZHYGXFVFTGRHQIOI"
    "MGK@@HYGWGUHSIRJPJNILEJIHJFJDIBHAG?G=H;@@GIIGIEHCGBF@F>G<H;J:","CIEZE:","hOFZH"
    "YIXJVJTIRHQGOGMIK@@HYIWIUHSGRFPFNGLKJGHFFFDGBHAI?I=H;@@IIGGGEHCIBJ@J>I<H;F:",""
    "XYDGDIELGMIMKLOIQHSHUIVK@@DIEKGLILKKOHQGSGUHVKVM" };
    vec3 src = pos, old = {0}; float abs_scale = absf(scale);
    for( signed char c; (c = *text++, c > 0 && c < 127); ) {
        if( c == '\n' || c == '\r' ) {
            pos.x = src.x, pos.y -= scale * ((signed char)hershey['W'-32][1] - 65) * 1.25f; // spacing @1
        } else {
            const char *glyph = (const char*)hershey[c - 32];
            if( c > 32 ) for( int pen = 0, i = 0; i < (glyph[0] - 65); i++ ) { // verts @0
                int x = glyph[2 + i*2 + 0] - 65, y = glyph[2 + i*2 + 1] - 65;
                if( x == -1 && y == -1 ) pen = 0; else {
                    vec3 next = add3(pos, vec3(abs_scale*x, scale*y, 0));
                    if( !pen ) pen = 1; else ddraw_line(old, next);
                    old = next;
                }
            }
            pos.x += abs_scale * (glyph[1] - 65); // spacing @1
        }
    }
}

void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments) {
    vec3 left = {0}, up = {0};
    ortho3(&left, &up, normal);

    vec3 point, lastPoint;
    up = scale3(up, radius);
    left = scale3(left, radius);
    lastPoint = add3(center, up);
    vec3 pivot = add3(center, scale3(normal, height));

    for (int i = 1; i <= segments; ++i) {
        const float radians = (C_PI * 2) * i / segments;

        vec3 vs = scale3(left, sinf(radians));
        vec3 vc = scale3(up,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        if( height > 0 ) ddraw_line(point, pivot);
        else if(height < 0) {
            ddraw_line(point, add3(point,scale3(normal, -height)));
        }
        lastPoint = point;
    }

    if(height < 0) ddraw_prism(add3(center, scale3(normal, -height)), radius, 0, normal, segments);
}

void ddraw_cube(vec3 center, float radius) { // draw_prism(center, 1, -1, vec3(0,1,0), 4);
    float half = radius * 0.5f;
    vec3 l = vec3(center.x-half,center.y+half,center.z-half); // left-top-far
    vec3 r = vec3(center.x+half,center.y-half,center.z+half); // right-bottom-near

    ddraw_line(l, vec3(r.x,l.y,l.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,l.y,r.z));
    ddraw_line(vec3(r.x,l.y,r.z), vec3(l.x,l.y,r.z));
    ddraw_line(vec3(l.x,l.y,r.z), l);
    ddraw_line(l, vec3(l.x,r.y,l.z));

    ddraw_line(r, vec3(l.x,r.y,r.z));
    ddraw_line(vec3(l.x,r.y,r.z), vec3(l.x,r.y,l.z));
    ddraw_line(vec3(l.x,r.y,l.z), vec3(r.x,r.y,l.z));
    ddraw_line(vec3(r.x,r.y,l.z), r);
    ddraw_line(r, vec3(r.x,l.y,r.z));

    ddraw_line(vec3(l.x,l.y,r.z), vec3(l.x,r.y,r.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,r.y,l.z));
}

#if 0 // @fixme: broken
void ddraw_cube44(vec3 radius, mat44 M) {
    float m33[9]; extract33(m33, M); // = { M[0,1,2], M[4,5,6], M[8,9,10] }    
    ddraw_cube33( vec3(M[12], M[13], M[14]), radius, m33 );
}
#endif

void ddraw_cube33(vec3 center, vec3 radius, mat33 M) {
    vec3 half = scale3(radius, 0.5f);
    vec3 l = vec3(-half.x,+half.y,-half.z); // left-top-far
    vec3 r = vec3(+half.x,-half.y,+half.z); // right-bottom-near

    vec3 points[8] = {
        vec3(l.x, r.y, r.z),
        vec3(l.x, r.y, l.z),
        vec3(r.x, r.y, l.z),
        vec3(r.x, r.y, r.z),
        vec3(l.x, l.y, r.z),
        vec3(l.x, l.y, l.z),
        vec3(r.x, l.y, l.z),
        vec3(r.x, l.y, r.z),
    };

    for( int i = 0; i < 8; ++i ) {
        points[i] = add3(center, transform33(M, points[i]));
    }

    ddraw_bounds(points);
}

void ddraw_normal(vec3 pos, vec3 n) {
    ddraw_color(YELLOW);
    ddraw_line(pos, add3(pos, norm3(n)));
}

void ddraw_circle(vec3 pos, vec3 n, float r) { ddraw_prism(pos, r, 0, n, 32); }
void ddraw_ring(vec3 pos, vec3 n, float r) { ddraw_circle(pos,n,r);ddraw_circle(pos,n,r*0.90); }
void ddraw_hexagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 6); }
void ddraw_pentagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 5); }
void ddraw_square(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 4); }
//void ddraw_triangle(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 3); }
void ddraw_sphere(vec3 center, float radius) {
    float lod = 8, yp = -radius, rp = 0, y, r, x, z;
    for( int j = 1; j <= lod / 2; ++j, yp = y, rp = r ) {
        y = j * 2.f / (lod / 2) - 1;
        r = cosf(y * 3.14159f / 2) * radius;
        y = sinf(y * 3.14159f / 2) * radius;

        float xp = 1, zp = 0;
        for( int i = 1; i <= lod; ++i, xp = x, zp = z ) {
            x = 3.14159f * 2 * i / lod;
            z = sinf(x);
            x = cosf(x);

            vec3 a1 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b1 = add3(center, vec3(xp * r,  y,  zp * r));
            vec3 c1 = add3(center, vec3(x  * r,  y,  z  * r));

            ddraw_line(a1,b1);
            ddraw_line(b1,c1);
            ddraw_line(c1,a1);

            vec3 a2 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b2 = add3(center, vec3(x  * r,  y,  z  * r));
            vec3 c2 = add3(center, vec3(x  * rp, yp, z  * rp));

            ddraw_line(a2,b2);
            ddraw_line(b2,c2);
            ddraw_line(c2,a2);
        }
    }
}
void ddraw_box(vec3 c, vec3 extents) {
    vec3 points[8], whd = scale3(extents, 0.5f);
    #define DD_BOX_V(v, op1, op2, op3) (v).x = c.x op1 whd.x; (v).y = c.y op2 whd.y; (v).z = c.z op3 whd.z
    DD_BOX_V(points[0], -, +, +);
    DD_BOX_V(points[1], -, +, -);
    DD_BOX_V(points[2], +, +, -);
    DD_BOX_V(points[3], +, +, +);
    DD_BOX_V(points[4], -, -, +);
    DD_BOX_V(points[5], -, -, -);
    DD_BOX_V(points[6], +, -, -);
    DD_BOX_V(points[7], +, -, +);
    #undef DD_BOX_V
    ddraw_bounds(points);
}
void ddraw_capsule(vec3 from, vec3 to, float r) {
    /* calculate axis */
    vec3 up, right, forward;
    forward = sub3(to, from);
    forward = norm3(forward);
    ortho3(&right, &up, forward);

    /* calculate first two cone verts (buttom + top) */
    vec3 lastf, lastt;
    lastf = scale3(up,r);
    lastt = add3(to,lastf);
    lastf = add3(from,lastf);

    /* step along circle outline and draw lines */
    enum { step_size = 20 };
    for (int i = step_size; i <= 360; i += step_size) {
        /* calculate current rotation */
        vec3 ax = scale3(right, sinf(i*TO_RAD));
        vec3 ay = scale3(up, cosf(i*TO_RAD));

        /* calculate current vertices on cone */
        vec3 tmp = add3(ax, ay);
        vec3 pf = scale3(tmp, r);
        vec3 pt = scale3(tmp, r);

        pf = add3(pf, from);
        pt = add3(pt, to);

        /* draw cone vertices */
        ddraw_line(lastf, pf);
        ddraw_line(lastt, pt);
        ddraw_line(pf, pt);

        lastf = pf;
        lastt = pt;

        /* calculate first top sphere vert */
        vec3 prevt = scale3(tmp, r);
        vec3 prevf = add3(prevt, from);
        prevt = add3(prevt, to);

        /* sphere (two half spheres )*/
        for (int j = 1; j < 180/step_size; j++) {
            /* angles */
            float ta = j*step_size;
            float fa = 360-(j*step_size);

            /* top half-sphere */
            ax = scale3(forward, sinf(ta*TO_RAD));
            ay = scale3(tmp, cosf(ta*TO_RAD));

            vec3 t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, to);
            ddraw_line(pf, prevt);
            prevt = pf;

            /* bottom half-sphere */
            ax = scale3(forward, sinf(fa*TO_RAD));
            ay = scale3(tmp, cosf(fa*TO_RAD));

            t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, from);
            ddraw_line(pf, prevf);
            prevf = pf;
        }
    }
}
void ddraw_pyramid(vec3 center, float height, int segments) {
    ddraw_prism(center, 1, height, vec3(0,1,0), segments);
}
void ddraw_cylinder(vec3 center, float height, int segments) {
    ddraw_prism(center, 1, -height, vec3(0,1,0), segments);    
}
void ddraw_diamond(vec3 from, vec3 to, float size) {
    poly p = diamond(from, to, size);
    vec3 *dmd = p.verts;

    vec3 *a = dmd + 0;
    vec3 *b = dmd + 1;
    vec3 *c = dmd + 2;
    vec3 *d = dmd + 3;
    vec3 *t = dmd + 4;
    vec3 *f = dmd + 5;

    /* draw vertices */
    ddraw_line(*a, *b);
    ddraw_line(*b, *c);
    ddraw_line(*c, *d);
    ddraw_line(*d, *a);

    /* draw roof */
    ddraw_line(*a, *t);
    ddraw_line(*b, *t);
    ddraw_line(*c, *t);
    ddraw_line(*d, *t);

    /* draw floor */
    ddraw_line(*a, *f);
    ddraw_line(*b, *f);
    ddraw_line(*c, *f);
    ddraw_line(*d, *f);

    poly_free(&p);
}
void ddraw_cone(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 24);
}
void ddraw_cone_lowres(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 3);
}
void ddraw_bone(vec3 center, vec3 end) {
    vec3 diff3 = sub3(end, center);
    float len = len3(diff3), len10 = len / 10;
    ddraw_prism(center, len10, 0, vec3(1,0,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,1,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,0,1), 24);
    ddraw_line(end, add3(center, vec3(0,+len10,0)));
    ddraw_line(end, add3(center, vec3(0,-len10,0)));
}
void ddraw_bounds(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        ddraw_line(points[i], points[(i + 1) & 3]);
        ddraw_line(points[i], points[4 + i]);
        ddraw_line(points[4 + i], points[4 + ((i + 1) & 3)]);
    }
}
void ddraw_bounds_corners(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        #define ddraw_unit(a,b) ddraw_line(a,add3(a,norm3(sub3(b,a)))), ddraw_line(b,add3(b,norm3(sub3(a,b))))
        ddraw_unit(points[i], points[(i + 1) & 3]);
        ddraw_unit(points[i], points[4 + i]);
        ddraw_unit(points[4 + i], points[4 + ((i + 1) & 3)]);
        #undef ddraw_unit
    }
}
void ddraw_aabb(vec3 minbb, vec3 maxbb) {
    vec3 points[8], bb[2] = { minbb, maxbb };
    for (int i = 0; i < 8; ++i) {
        points[i].x = bb[(i ^ (i >> 1)) & 1].x;
        points[i].y = bb[     (i >> 1)  & 1].y;
        points[i].z = bb[     (i >> 2)  & 1].z;
    }
    ddraw_bounds/*_corners*/(points);
}
void ddraw_frustum(float projview[16]) {
    mat44 clipmatrix = {0}; // clip matrix
    invert44(clipmatrix, projview);

    // Start with the standard clip volume, then bring it back to world space.
    const vec3 planes[8] = {
        {-1,-1,-1}, {+1,-1,-1}, {+1,+1,-1}, {-1,+1,-1}, // near plane
        {-1,-1,+1}, {+1,-1,+1}, {+1,+1,+1}, {-1,+1,+1}, // far plane
    };

    vec3 points[8];
    float wCoords[8];

    // Transform the planes by the inverse clip matrix:
    for( int i = 0; i < 8; ++i ) {
        // wCoords[i] = matTransformPointXYZW2(&points[i], planes[i], clipmatrix);
        vec3 *out = &points[i], in = planes[i]; const float *m = clipmatrix;
        out->x = (m[0] * in.x) + (m[4] * in.y) + (m[ 8] * in.z) + m[12]; // in.w (vec4) assumed to be 1
        out->y = (m[1] * in.x) + (m[5] * in.y) + (m[ 9] * in.z) + m[13];
        out->z = (m[2] * in.x) + (m[6] * in.y) + (m[10] * in.z) + m[14];
        wCoords[i] =  (m[3] * in.x) + (m[7] * in.y) + (m[11] * in.z) + m[15]; // rw

        // bail if any W ended up as zero.
        const float epsilon = 1e-9f;
        if (absf(wCoords[i]) < epsilon) {
            return;
        }
    }

    // Divide by the W component of each:
    for( int i = 0; i < 8; ++i ) {
        points[i].x /= wCoords[i];
        points[i].y /= wCoords[i];
        points[i].z /= wCoords[i];
    }

    // Connect the dots:
    ddraw_bounds(points);
}
void ddraw_arrow(vec3 begin, vec3 end) {
    vec3 diff = sub3(end, begin);
    float len = len3(diff), stick_len = len * 2 / 3;

    ddraw_line(begin, end);
    ddraw_cone_lowres(add3(begin, scale3(norm3(diff), stick_len)), end, len / 6);
}

void ddraw_plane(vec3 p, vec3 n, float scale) {
    // if n is too similar to up vector, use right. else use up vector
    vec3 v1 = cross3(n, dot3(n, vec3(0,1,0)) > 0.8f ? vec3(1,0,0) : vec3(0,1,0));
    vec3 v2 = cross3(n, v1);

    // draw axis
    ddraw_line(p, add3(p,n));
    ddraw_line(p, add3(p,v1));
    ddraw_line(p, add3(p,v2));

    // get plane coords
    v1 = scale3(v1, scale);
    v2 = scale3(v2, scale);
    vec3 p1 = add3(add3(p, v1), v2);
    vec3 p2 = add3(sub3(p, v1), v2);
    vec3 p3 = sub3(sub3(p, v1), v2);
    vec3 p4 = sub3(add3(p, v1), v2);

    // draw plane
    ddraw_line(p1, p2);
    ddraw_line(p2, p3);
    ddraw_line(p3, p4);
    ddraw_line(p4, p1);
}

void ddraw_boid(vec3 position, vec3 dir) {
    dir = norm3(dir);

    // if n is too similar to up vector, use right. else use up vector
    vec3 v1 = cross3(dir, dot3(dir, vec3(0,1,0)) > 0.8f ? vec3(1,0,0) : vec3(0,1,0));
    vec3 v2 = cross3(dir, v1);
    v1 = cross3(dir, v2);

    uint32_t bak = dd_color;
    ddraw_color( position.y < 0 ? ORANGE : CYAN );

        vec3 front = add3(position, scale3(dir, 1));
        vec3 back = add3(position, scale3(dir, -0.25f));
        vec3 right = add3(back, scale3(v1, 0.5f));
        vec3 left = add3(back, scale3(v1, -0.5f));
        ddraw_line( front, left );
        ddraw_line( left, position );
        ddraw_line( position, right );
        ddraw_line( right, front );

    dd_color = bak;
}

void ddraw_circle__with_orientation(vec3 center, vec3 dir, float radius) {
    // we'll skip 3 segments out of 32. 1.5 per half circle.
    int segments = 32, skip = 3, drawn_segments = segments-skip;

//  dir = norm3(dir);
    vec3 right = cross3(dir, vec3(0,1,0));
    vec3 up = cross3(dir, right);
    right = cross3(dir, up);

    vec3 point, lastPoint;
    dir = scale3(dir, radius);
    right = scale3(right, radius);
    //lastPoint = add3(center, dir);

    {
        const float radians = (C_PI * 2) * (0+skip/2.f) / segments;
        vec3 vs = scale3(right, sinf(radians));
        vec3 vc = scale3(dir,   cosf(radians));
        lastPoint = add3(center, vs);
        lastPoint = add3(lastPoint, vc);
    }

    //ddraw_color(GREEN);
    ddraw_line(lastPoint, add3(center, scale3(dir, radius * (radius <= 1 ? 1.25 : 0.65) )));
    //ddraw_color(YELLOW);

    for (int i = 0; i <= drawn_segments; ++i) {
        const float radians = (C_PI * 2) * (i+skip/2.f) / segments;

        vec3 vs = scale3(right, sinf(radians));
        vec3 vc = scale3(dir,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        lastPoint = point;
    }

    //ddraw_color(RED);
    ddraw_line(lastPoint, add3(center, scale3(dir, radius * (radius <= 1 ? 1.25 : 0.65) )));
}

void ddraw_position_dir( vec3 position, vec3 direction, float radius ) {
    // idea from http://www.cs.caltech.edu/~keenan/m3drv.pdf and flotilla game UI
    uint32_t bak = dd_color;

    vec3 ground = vec3(position.x, 0, position.z);
    ddraw_color( position.y < 0 ? PINK/*ORANGE*/ : CYAN );
    ddraw_point( ground );
    ddraw_point( position );
    (position.y < 0 ? ddraw_line_dashed : ddraw_line)( ground, position );

    vec3 n = norm3(direction), up = vec3(0,1,0);
    for( int i = 0; i < 10 && i <= fabs(position.y); ++i ) {
        if( i < 2 && len3(direction) )
        ddraw_circle__with_orientation(ground, n, radius);
        else
        ddraw_circle(ground, up, radius);
        radius *= 0.9f;
    }

    dd_color = bak;
}

void ddraw_position( vec3 position, float radius ) {
    ddraw_position_dir(position, vec3(0,0,0), radius);
}

void ddraw_init() {
    do_once {
    for( int i = 0; i < 2; ++i )
    for( int j = 0; j < 3; ++j ) map_init(dd_lists[i][j], less_int, hash_int);
    dd_program = shader(dd_vs,dd_fs,"att_position","fragcolor");
    dd_u_color = glGetUniformLocation(dd_program, "u_color");
    ddraw_flush(); // alloc vao & vbo, also resets color
    }
}

void ddraw_demo() {
    ddraw_color_push(YELLOW);

    // freeze current frame for (frustum) camera forensics
    static mat44 projview_copy;
    do_once {
        multiply44x2(projview_copy, camera_get_active()->proj, camera_get_active()->view);
    }
    ddraw_frustum(projview_copy);

    //ddraw_grid();

    vec3 origin = {0,0,0};
    ddraw_color(ORANGE);
    ddraw_arrow(origin, vec3(-1,1,1));
    ddraw_color(YELLOW);
    ddraw_text(vec3(-1,1,1), 0.008f, va("hello 1%s2!", "world"));

    const char abc[] = " !\"#$%&'()*+,-./\n"
        "0123456789:;<=>?@\n"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`\n"
        "abcdefghijklmnopqrstuvwxyz{|}~";
    ddraw_text(vec3(2,2,2), 0.008f, abc);


    for( int i = -5; i <= 5; ++i ) {
        ddraw_pyramid(vec3(i*2,0,3),  0, i+5+2); ddraw_text(vec3(i*2,0,3), 0.008f, va("%d/1", i));
        ddraw_pyramid(vec3(i*2,0,6), -2, i+5+2); ddraw_text(vec3(i*2,0,6), 0.008f, va("%d/2", i));
        ddraw_pyramid(vec3(i*2,0,9), +2, i+5+2); ddraw_text(vec3(i*2,0,9), 0.008f, va("%d/3", i));
    }

#if 1 // @fixme: add positions to these
    // ddraw_triangle(origin, 1);
    ddraw_square(origin, 1);
    ddraw_pentagon(origin, 1);
    ddraw_hexagon(origin, 1);
    ddraw_cube(origin, 1);
    ddraw_pyramid(origin, 2, 3);
    ddraw_pyramid(origin, 2, 16);
    ddraw_cone(origin, add3(origin, vec3(0,1,0)), 0.5f);
    ddraw_arrow(origin, vec3(0,1,0));
    ddraw_bone(vec3(0,0,0), vec3(3,3,3));
    ddraw_aabb(vec3(0,0,0), vec3(1,1,1));
#endif

    ddraw_plane(vec3(0,10,0), vec3(0,1,0), 10);
    //ddraw_boid(vec3(15,0,15), vec3(-15,0,-15) );
    ddraw_position(vec3(10,10,10), 1);
    ddraw_position(vec3(-10,-10,10), 1);

    ddraw_point(vec3(-2,0,-2));
    ddraw_color(PURPLE);
    ddraw_sphere(vec3(-3,0,-3),1);

    ddraw_color_pop();
}
