uniform float CellSize;  /// min:1 set:2.5 max:16
//uniform float xCellSize; /// min:1 set:2.5
//uniform float yCellSize; /// min:1 set:2.5

void main() {
    float xPixels = iWidth/CellSize, yPixels = iHeight/CellSize; //  iWidth/xCellSize, iHeight/yCellSize;
    vec2 uv = vec2(floor(texcoord.s * xPixels) / xPixels, floor(texcoord.t * yPixels) / yPixels);
    FRAGCOLOR = texture(iChannel0, uv);
}
