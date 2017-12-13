/*!
\file
* \brief Source file for the class BumpmapShader.
*
* $Author: emcha1/pfafd1 $
* $Date: 2013-01-12 $
* $Revision: 1 $
*/

#include "SunShader.h"

#include <osg/Node>
#include <osg/StateSet>
#include <osg/Program>
#include "../../config.h"

using namespace pbs17;



SunShader::SunShader(osg::ref_ptr<osg::Texture2D> texture, osg::ref_ptr<osg::Texture2D> normals)
	: _texture(texture), _normals(normals) {
	setVertShader(
		"attribute vec3 tangent;\n"
		"attribute vec3 binormal;\n"
		"varying vec3 lightDir;\n"
		"varying vec3 position;\n"
		"void main()\n"
		"{\n"
		"    vec3 normal = normalize(gl_NormalMatrix * gl_Normal);\n"
		"    mat3 rotation = mat3(tangent, binormal, normal);\n"
		"    vec4 vertexInEye = gl_ModelViewMatrix * gl_Vertex;\n"
		"    lightDir = vec3(gl_LightSource[0].position.xyz - vertexInEye.xyz);\n"
		"    lightDir = normalize(normalize(lightDir) * rotation);\n"
		"    gl_Position = ftransform();\n"
		"	 position = gl_Vertex.xyz;\n"
		"    gl_TexCoord[0] = gl_MultiTexCoord0;\n"
		"}\n"
	);

	setFragShader(
		"uniform sampler2D colorTex;\n"
		"uniform sampler2D normalTex;\n"
		"uniform vec3 CAMERA_POSITION;\n"
		"varying vec3 lightDir;\n"
		"varying vec3 position;\n"

		"void main (void)\n"
		"{\n"
		"    vec4 base = texture2D(colorTex, gl_TexCoord[0].xy);\n"
		"    vec3 bump = texture2D(normalTex, gl_TexCoord[0].xy).xyz;\n"
		"    bump = normalize(bump * 2.0 - 1.0);\n"

		"    float lambert = abs(dot(bump, lightDir));\n"
		"	 gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
		"	 vec4 diffuse = vec4(0.0, 0.0, 0.0, 0.0);\n"
		"	 vec4 specular = vec4(0.0, 0.0, 0.0, 0.0);\n"
		"	 vec4 ambient = base * gl_FrontMaterial.ambient * gl_LightSource[0].ambient;\n"

		"    if (lambert > 0.0)\n"
		"    {\n"
		"        diffuse = gl_FrontMaterial.diffuse * base * gl_LightSource[0].diffuse * lambert;\n"
		"        specular = gl_FrontMaterial.specular * gl_LightSource[0].specular * pow(lambert, gl_FrontMaterial.shininess);\n"
		"    }\n"
		"    gl_FragColor += ambient + diffuse + specular;\n"
		"}\n"
	);
}



SunShader::~SunShader() {}



void SunShader::apply(osg::Node* node) {
	//osg::ref_ptr<osg::Shader> fragShader = new osg::Shader(osg::Shader::FRAGMENT);
	//fragShader->loadShaderSourceFromFile(DATA_PATH + "/sunShader.glsl");

	osg::ref_ptr<osg::Program> program = new osg::Program;
	program->addShader(new osg::Shader(osg::Shader::VERTEX, getVertShader()));
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, getFragShader()));
	program->addBindAttribLocation("random_noise", 6);
	program->addBindAttribLocation("binormal", 7);

	osg::StateSet* stateset = node->getOrCreateStateSet();
	stateset->setAttributeAndModes(program.get());
	stateset->addUniform(new osg::Uniform("colorTex", 0));
	stateset->addUniform(new osg::Uniform("random_noise", 7));

	osg::Uniform* uniform = stateset->getOrCreateUniform("iTime", osg::Uniform::FLOAT);
	uniform->set(static_cast<float>(5.0));

	uniform = stateset->getOrCreateUniform("iMouse", osg::Uniform::FLOAT_VEC2);
	uniform->set(osg::Vec2(0.5, 0.5));
	//stateset->addUniform(new osg::Uniform("normalTex", 1));

	osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE;
	stateset->setTextureAttributeAndModes(0, _texture.get(), value);
	stateset->setTextureAttributeAndModes(1, _normals.get(), value);

}