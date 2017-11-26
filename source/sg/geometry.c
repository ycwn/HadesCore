

#include "core/system.h"
#include "core/types.h"
#include "core/debug.h"
#include "core/blob.h"
#include "core/list.h"
#include "core/logger.h"
#include "core/variable.h"

#include "gr/limits.h"
#include "gr/graphics.h"
#include "gr/rendertarget.h"
#include "gr/vertexformat.h"
#include "gr/vertexbuffer.h"
#include "gr/uniformbuffer.h"
#include "gr/shader.h"
#include "gr/command.h"
#include "gr/commandqueue.h"
#include "gr/commandlist.h"
#include "gr/geometry.h"

#include "sg/transform.h"
#include "sg/scenegraph.h"
#include "sg/entity.h"
#include "sg/geometry.h"



sg_geometry *sg_geometry_new(const char *name)
{

	sg_geometry *g = (sg_geometry*)sg_entity_new(name, sizeof(sg_geometry) - sizeof(sg_entity));

	gr_geometry_init(&g->geometry);

	return g;

}



void sg_geometry_del(sg_geometry *g)
{

	sg_entity_del(&g->entity);

}



bool sg_geometry_load(sg_geometry *g, const char *file)
{

	return sg_geometry_loadfd(g, blob_open(file, BLOB_REV_LAST));

}



bool sg_geometry_loadfd(sg_geometry *g, int id)
{

	if (!gr_geometry_openfd(&g->geometry, id))
		return false;

	gr_geometry_commit_drawcalls(&g->geometry, &g->entity.cmds);
	gr_geometry_commit_buffers(  &g->geometry, &g->entity.vbo);

	g->entity.recalculate = true;

	return true;

}

