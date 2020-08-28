// tgl_init.cpp 

#include "tgl.h"

GLContext *gl_ctx = NULL;

GLContext *gl_get_context()
{
	return gl_ctx;
} 

void initSharedState(GLContext *c)
{
	GLSharedState *s = &c->shared_state;
	s->texture_hash_table = (GLTexture**) gl_zalloc(sizeof(GLTexture *) * TEXTURE_HASH_TABLE_SIZE);
	alloc_texture(c,0);
}

void endSharedState(GLContext *c)
{
	GLSharedState *s = &c->shared_state;

	// anchor 2017.02.25
	while ( find_texture(c,0) )
	{
		free_texture(c,0);
	}

	gl_free(s->texture_hash_table);
}

void glInit(void *zbuffer1)
{
	ZBuffer *zbuffer = (ZBuffer *)zbuffer1;
	GLContext *c;
	GLViewport *v;
	int i;

	c = (GLContext*) gl_zalloc(sizeof(GLContext));
	gl_ctx = c;

	c->zb = zbuffer;

	/* allocate GLVertex array */
	c->vertex_max = POLYGON_MAX_VERTEX;
	c->vertex = (GLVertex*) gl_malloc(POLYGON_MAX_VERTEX*sizeof(GLVertex));

	/* viewport */
	v = &c->viewport;
	v->xmin = 0;
	v->ymin = 0;
	v->xsize = zbuffer->xsize;
	v->ysize = zbuffer->ysize;
	v->updated = 1;

	/* shared state */
	initSharedState(c);

	c->print_flag = 0;

	c->in_begin = 0;

	/* lights */
	for(i=0;i<MAX_LIGHTS;i++)
	{
		GLLight *l = &c->light.lights[i];
		l->ambient = gl_V4_New(0,0,0,1);
		l->diffuse = gl_V4_New(1,1,1,1);
		l->specular = gl_V4_New(1,1,1,1);
		l->position = gl_V4_New(0,0,1,0);
		l->norm_position = gl_V3_New(0,0,1);
		l->spot_direction = gl_V3_New(0,0,-1);
		l->norm_spot_direction = gl_V3_New(0,0,-1);
		l->spot_exponent = 0;
		l->spot_cutoff = 180;
		l->attenuation[0] = 1;
		l->attenuation[1] = 0;
		l->attenuation[2] = 0;
		l->enabled = 0;
	}
	c->light.first = NULL;
	c->light.model.ambient = gl_V4_New(.2f,.2f,.2f,1.f);
	c->light.model.local = 0;
	c->light.enabled = 0;
	c->light.model.two_side = 0;

	/* default materials */
	for(i=0;i<2;i++)
	{
		GLMaterial *m = &c->material.materials[i];
		m->emission = gl_V4_New(0.f,0.f,0.f,1.f);
		m->ambient = gl_V4_New(.2f,.2f,.2f,1.f);
		m->diffuse = gl_V4_New(.8f,.8f,.8f,1.f);
		m->specular = gl_V4_New(0.f,0.f,0.f,1.f);
		m->shininess = 0;
	}
	c->material.color.current_mode = GL_FRONT_AND_BACK;
	c->material.color.current_type = GL_AMBIENT_AND_DIFFUSE;
	c->material.color.enabled = 0;

	/* textures */
	gl_init_textures(c);

	/* default state */
	c->current.color.X = 1.0;
	c->current.color.Y = 1.0;
	c->current.color.Z = 1.0;
	c->current.color.W = 1.0;
	c->current.longcolor[0] = 65535;
	c->current.longcolor[1] = 65535;
	c->current.longcolor[2] = 65535;

	c->current.normal.X = 1.0;
	c->current.normal.Y = 0.0;
	c->current.normal.Z = 0.0;
	c->current.normal.W = 0.0;

	c->current.edge_flag = 1;

	c->current.tex_coord.X = 0;
	c->current.tex_coord.Y = 0;
	c->current.tex_coord.Z = 0;
	c->current.tex_coord.W = 1;

	c->polygon_mode_front = GL_FILL;
	c->polygon_mode_back = GL_FILL;

	c->current_front_face = 0; /* 0 = GL_CCW  1 = GL_CW */
	c->current_cull_face = GL_BACK;
	c->current_shade_model = GL_SMOOTH;
	c->cull_face_enabled = 0;

	/* clear */
	c->clear.color.X = 0;
	c->clear.color.Y = 0;
	c->clear.color.Z = 0;
	c->clear.color.W = 0;
	c->clear.depth = 0;

	/*
	c->render_mode = GL_RENDER;
	c->select_buffer = NULL;
	c->name_stack_size = 0;
	*/

	/* matrix */
	c->matrix.mode = 0;

	c->matrix.stack_depth_max[0] = MAX_MODELVIEW_STACK_DEPTH;
	c->matrix.stack_depth_max[1] = MAX_PROJECTION_STACK_DEPTH;
	c->matrix.stack_depth_max[2] = MAX_TEXTURE_STACK_DEPTH;

	for(i=0;i<3;i++)
	{
		c->matrix.stack[i] = (M4*) gl_zalloc(c->matrix.stack_depth_max[i] * sizeof(M4));
		c->matrix.stack_ptr[i] = c->matrix.stack[i];
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	c->matrix.model_projection_updated = 1;

	/* opengl 1.1 arrays */
	c->client_states = 0;

	/* opengl 1.1 polygon offset */
	c->offset.states = 0;

	/* clear the resize callback function pointer */
	c->gl_resize_viewport = NULL;

	/* specular buffer */
	c->specbuf_first = NULL;
	c->specbuf_used_counter = 0;
	c->specbuf_num_buffers = 0;

	/* depth test */
	c->depth_test = 0;
}

void glClose()
{
	int i;
	GLContext *c = gl_get_context();

	endSharedState(c);

	// anchor 2017.02.25
	for(i=0;i<3;i++)
	{
		gl_free(c->matrix.stack[i]);
	}

	gl_free(c->vertex); // anchor 2017.02.25

	gl_free(c);
}
