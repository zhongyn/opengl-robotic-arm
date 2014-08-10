varying vec4 texCoords;
uniform sampler2D mytexture;


void main()
{
	vec2 longitudeLatitude = vec2(1-(atan(texCoords.z, texCoords.y) / 3.1415926 + 1.0) * 0.5,
                                  1-(asin(texCoords.x) / 3.1415926 + 0.5));	
	gl_FragColor = texture2D(mytexture, longitudeLatitude);
}