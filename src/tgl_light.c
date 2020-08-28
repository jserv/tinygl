// tgl_light.cpp

#include "tgl.h"

void glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
	GLfloat v[4] = {param, 0, 0, 0};
	glMaterialfv(face, pname, v);
}

void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
	GLContext *c = gl_get_context();
	GLMaterial *m;

	if ( face == GL_FRONT_AND_BACK )
	{
		glMaterialfv(GL_FRONT,pname,params);
		face = GL_BACK;
	}
	if ( face == GL_FRONT ) 
	{
		m = &c->material.materials[0];
	}
	else
	{
		m = &c->material.materials[1];
	}

	switch ( pname )
	{
	case GL_EMISSION:
		m->emission.X = params[0];
		m->emission.Y = params[1];
		m->emission.Z = params[2];
		m->emission.W = params[3];
		break;
	case GL_AMBIENT:
		m->ambient.X = params[0];
		m->ambient.Y = params[1];
		m->ambient.Z = params[2];
		m->ambient.W = params[3];
		break;
	case GL_DIFFUSE:
		m->diffuse.X = params[0];
		m->diffuse.Y = params[1];
		m->diffuse.Z = params[2];
		m->diffuse.W = params[3];
		break;
	case GL_SPECULAR:
		m->specular.X = params[0];
		m->specular.Y = params[1];
		m->specular.Z = params[2];
		m->specular.W = params[3];
		break;
	case GL_SHININESS:
		m->shininess = params[0];
		m->shininess_i = (int) ((params[0]/128.0f)*SPECULAR_BUFFER_RESOLUTION);
		break;
	case GL_AMBIENT_AND_DIFFUSE:
		m->diffuse.X = params[0];
		m->diffuse.Y = params[1];
		m->diffuse.Z = params[2];
		m->diffuse.W = params[3];
		m->ambient.X = params[0];
		m->ambient.Y = params[1];
		m->ambient.Z = params[2];
		m->ambient.W = params[3];
		break;
	default:
		tgl_warning("%s invalid param",__FUNCTION__);
	}
}

void glColorMaterial(GLenum face, GLenum mode) 
{
	GLContext *c = gl_get_context();
	c->material.color.current_mode = face;
	c->material.color.current_type = mode;
}

void glLightf(GLenum light, GLenum pname, GLfloat param)
{
	float v[4] = {param, 0, 0, 0};
	glLightfv(light, pname, v);
}

// TODO: 3 components?
void glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
	GLContext *c = gl_get_context();
	GLLight *l;
	V4 v;

	assert(light >= GL_LIGHT0 && light < GL_LIGHT0+MAX_LIGHTS );

	l = &c->light.lights[light-GL_LIGHT0];

	v = *(V4 *)params;

	switch ( pname )
	{
	case GL_AMBIENT:
		l->ambient = v;
		break;
	case GL_DIFFUSE:
		l->diffuse = v;
		break;
	case GL_SPECULAR:
		l->specular = v;
		break;
	case GL_POSITION:
		{
			V4 pos;
			gl_M4_MulV4(&pos,c->matrix.stack_ptr[0],&v);

			l->position = pos;

			if ( l->position.W == 0 )
			{
				l->norm_position.X = pos.X;
				l->norm_position.Y = pos.Y;
				l->norm_position.Z = pos.Z;

				gl_V3_Norm(&l->norm_position);
			}
		}
		break;
	case GL_SPOT_DIRECTION:
		l->spot_direction.X = v.X;
		l->spot_direction.Y = v.Y;
		l->spot_direction.Z = v.Z;
		l->norm_spot_direction.X = v.X;
		l->norm_spot_direction.Y = v.Y;
		l->norm_spot_direction.Z = v.Z;
		gl_V3_Norm(&l->norm_spot_direction);
		break;
	case GL_SPOT_EXPONENT:
		l->spot_exponent = v.X;
		break;
	case GL_SPOT_CUTOFF:
		{
			float a = v.X;
			assert(a == 180 || (a>=0 && a<=90));
			l->spot_cutoff = a;
			if ( a != 180 )
			{
				l->cos_spot_cutoff = (float) cos(a * M_PI / 180.0);
			}
		}
		break;
	case GL_CONSTANT_ATTENUATION:
		l->attenuation[0] = v.X;
		break;
	case GL_LINEAR_ATTENUATION:
		l->attenuation[1] = v.X;
		break;
	case GL_QUADRATIC_ATTENUATION:
		l->attenuation[2] = v.X;
		break;
	default:
		tgl_warning("%s invalid param",__FUNCTION__);
	}
}

void glLightModeli(GLenum pname, GLint param)
{
	GLfloat v[4] = { (GLfloat)param, 0, 0, 0};
	glLightModelfv(pname, v);
}

void glLightModelfv(GLenum pname, const GLfloat *param) 
{
	GLContext *c = gl_get_context();
	V4 v = *(V4 *)param;

	switch ( pname )
	{
	case GL_LIGHT_MODEL_AMBIENT:
		c->light.model.ambient = v;
		break;
	case GL_LIGHT_MODEL_LOCAL_VIEWER:
		c->light.model.local = (int)v.X;
		break;
	case GL_LIGHT_MODEL_TWO_SIDE:
		c->light.model.two_side = (int)v.X;
		break;
	default:
		tgl_warning("%s illegal pname: 0x%x\n",__FUNCTION__, pname);
		break;
	}
}

float clampf(float a, float min, float max)
{
	if ( a < min )
	{
		return min;
	}
	else if ( a > max )
	{
		return max;
	}
	return a;
}

/* non optimized lightening model */
void gl_shade_vertex(GLContext *c, GLVertex *v)
{
	float R,G,B,A;
	GLMaterial *m;
	GLLight *l;
	V3 n,s,d;
	float dist,tmp,att;
	float dot;
	float dot_spec;

	int twoside = c->light.model.two_side;

	m = &c->material.materials[0];

	n.X = v->normal.X;
	n.Y = v->normal.Y;
	n.Z = v->normal.Z;

	// this light calculation is born because the old code generates false colors.
	// and at least the gears example looks good now.
	// should be removed, when the real code fixed - anchor 2017.03.07
	s.X = v->ec.X;
	s.Y = v->ec.Y;
	s.Z = v->ec.Z;
	gl_V3_Norm(&s);
	//gl_V3_Norm(&n);
	dot = s.X*n.X + s.Y*n.Y + s.Z*n.Z;
	dot = (dot*dot)*2.f;

	v->color.X = clampf((m->diffuse.X*dot)*.5f+m->ambient.X*.75f,0,1);
	v->color.Y = clampf((m->diffuse.Y*dot)*.5f+m->ambient.Y*.75f,0,1);
	v->color.Z = clampf((m->diffuse.Z*dot)*.5f+m->ambient.Z*.75f,0,1);
	v->color.W = m->diffuse.W;

	/*

	//
	// this code need to be fixed
	//

	R = m->emission.X+m->ambient.X*c->light.model.ambient.X;
	G = m->emission.Y+m->ambient.Y*c->light.model.ambient.Y;
	B = m->emission.Z+m->ambient.Z*c->light.model.ambient.Z;
	A = clampf(m->diffuse.W,0,1);

	for(l=c->light.first;l!=NULL;l=l->next)
	{
		float lR,lB,lG;

		// ambient
		lR = l->ambient.X * m->ambient.X;
		lG = l->ambient.Y * m->ambient.Y;
		lB = l->ambient.Z * m->ambient.Z;

		if ( l->position.W == 0 )
		{
			// light at infinity
			d.X = l->position.X;
			d.Y = l->position.Y;
			d.Z = l->position.Z;
			att = 1;
		} 
		else
		{
			// distance attenuation
			d.X = l->position.X-v->ec.X;
			d.Y = l->position.Y-v->ec.Y;
			d.Z = l->position.Z-v->ec.Z;
			dist = sqrt(d.X*d.X+d.Y*d.Y+d.Z*d.Z);
			if ( dist>1E-3 )
			{
				tmp = 1/dist;
				d.X *= tmp;
				d.Y *= tmp;
				d.Z *= tmp;
			}
			att = 1.0f/(l->attenuation[0]+dist*(l->attenuation[1]+dist*l->attenuation[2]));
		}
		dot = d.X*n.X+d.Y*n.Y+d.Z*n.Z;
		if ( twoside && dot < 0 )
		{
			dot = -dot;
		}
		if ( dot > 0)
		{
			// diffuse light
			lR += dot * l->diffuse.X * m->diffuse.X;
			lG += dot * l->diffuse.Y * m->diffuse.Y;
			lB += dot * l->diffuse.Z * m->diffuse.Z;

			// spot light
			if ( l->spot_cutoff != 180 )
			{
				float dot_spot = -(d.X*l->norm_spot_direction.X+
					         d.Y*l->norm_spot_direction.Y+
					         d.Z*l->norm_spot_direction.Z);
				if ( twoside && dot_spot < 0 )
				{
					dot_spot = -dot_spot;
				}
				if ( dot_spot < l->cos_spot_cutoff )
				{
					// no contribution
					continue;
				}
				else
				{
					// TODO: optimize
					if ( l->spot_exponent > 0 )
					{
						att = att*pow(dot_spot,l->spot_exponent);
					}
				}
			}

			// specular light

			if ( c->light.model.local )
			{
				V3 vcoord;
				vcoord.X = v->ec.X;
				vcoord.Y = v->ec.Y;
				vcoord.Z = v->ec.Z;
				gl_V3_Norm(&vcoord);
				s.X = d.X-vcoord.X;
				s.Y = d.Y-vcoord.X;
				s.Z=  d.Z-vcoord.X;
			} 
			else
			{
				s.X = d.X;
				s.Y = d.Y;
				s.Z = d.Z+1.f;
			}
			dot_spec = n.X*s.X+n.Y*s.Y+n.Z*s.Z;
			if ( twoside && dot_spec < 0 )
			{
				dot_spec = -dot_spec;
			}
			if ( dot_spec > 0 )
			{
				GLSpecBuf *specbuf;
				int idx;
				tmp = sqrt(s.X*s.X+s.Y*s.Y+s.Z*s.Z);
				if ( tmp > 1E-3)
				{
					dot_spec = dot_spec / tmp;
				}

				// TODO: optimize
				// testing specular buffer code
				// dot_spec= pow(dot_spec,m->shininess);
				specbuf = specbuf_get_buffer(c, m->shininess_i, m->shininess);
				idx = (int)(dot_spec*SPECULAR_BUFFER_SIZE);
				if (idx > SPECULAR_BUFFER_SIZE) idx = SPECULAR_BUFFER_SIZE;
				dot_spec = specbuf->buf[idx];
				lR += dot_spec * l->specular.X * m->specular.X;
				lG += dot_spec * l->specular.Y * m->specular.Y;
				lB += dot_spec * l->specular.Z * m->specular.Z;
			}
		}

		R += att * lR;
		G += att * lG;
		B += att * lB;
	}

	v->color.X = clampf(R,0,1);
	v->color.Y = clampf(G,0,1);
	v->color.Z = clampf(B,0,1);
	v->color.W = A;
	*/
}
