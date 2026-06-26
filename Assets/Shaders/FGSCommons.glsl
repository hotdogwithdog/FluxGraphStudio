// Commons Code that are included automatically in all the gNodes files when they generate the declarations code

vec2 GetUV(ivec2 pixelCoord, ivec2 imgSize)
{
    return vec2(pixelCoord) / vec2(imgSize);
}