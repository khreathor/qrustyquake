#include <stdint.h>

#ifndef QTYPEDEFS_
#define QTYPEDEFS_
typedef uint8_t  u8;                                           // standard types
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;
typedef float    f32;
typedef double   f64;

typedef unsigned char byte; // TODO remove
typedef unsigned char pixel_t; // TODO remove

typedef f32   vec_t;                                                 // q_stdinc
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];
typedef s32   fixed4_t;
typedef s32   fixed8_t;
typedef s32   fixed16_t;

typedef struct {                                                      // bspfile
	s32 fileofs, filelen;
} lump_t;
typedef struct {
	f32 mins[3], maxs[3];
	f32 origin[3];
	s32 headnode[MAX_MAP_HULLS];
	s32 visleafs; // not including the solid leaf 0
	s32 firstface, numfaces;
} dmodel_t;
typedef struct {
	s32 version;
	lump_t lumps[HEADER_LUMPS];
} dheader_t;
typedef struct {
	s32 nummiptex;
	s32 dataofs[4]; // [nummiptex]
} dmiptexlump_t;
typedef struct miptex_s {
	s8 name[16];
	unsigned width, height;
	unsigned offsets[MIPLEVELS]; // four mip maps stored
} miptex_t;
typedef struct {
	f32 point[3];
} dvertex_t;
typedef struct {
	f32 normal[3];
	f32 dist;
	s32 type; // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} dplane_t;
typedef struct {
	s32 planenum;
	s16 children[2]; // negative numbers are -(leafs+1), not nodes
	s16 mins[3]; // for sphere culling
	s16 maxs[3];
	u16 firstface;
	u16 numfaces; // counting both sides
} dnode_t;
typedef struct {
	s32 planenum;
	s32 children[2]; // negative numbers are -(leafs+1), not nodes
	s16 mins[3]; // for sphere culling
	s16 maxs[3];
	u32 firstface;
	u32 numfaces; // counting both sides
} dl1node_t;
typedef struct {
	s32 planenum;
	s32 children[2]; // negative numbers are -(leafs+1), not nodes
	f32 mins[3]; // for sphere culling
	f32 maxs[3];
	u32 firstface;
	u32 numfaces; // counting both sides
} dl2node_t;
typedef struct {
	s32 planenum;
	s16 children[2]; // negative numbers are contents
} dclipnode_t;
typedef struct texinfo_s {
	f32 vecs[2][4]; // [s/t][xyz offset]
	s32 miptex;
	s32 flags;
} texinfo_t;
typedef struct { // note that edge 0 is never used, because negative edge nums are used for counterclockwise use of the edge in a face
	u16 v[2]; // vertex numbers
} dedge_t;
typedef struct {
	u32 v[2]; // vertex numbers
} dledge_t;
typedef struct {
	s16 planenum;
	s16 side;
	s32 firstedge; // we must support > 64k edges
	s16 numedges;
	s16 texinfo;
	byte styles[MAXLIGHTMAPS]; // lighting info
	s32 lightofs; // start of [numstyles*surfsize] samples
} dface_t;
typedef struct {
	s32 planenum;
	s32 side;
	s32 firstedge; // we must support > 64k edges
	s32 numedges;
	s32 texinfo;
	// lighting info
	byte styles[MAXLIGHTMAPS];
	s32 lightofs; // start of [numstyles*surfsize] samples
} dlface_t;
typedef struct { // leaf 0 is the generic CONTENTS_SOLID leaf, used for all solid areas, all other leafs need visibility info
	s32 contents;
	s32 visofs; // -1 = no visibility info
	s16 mins[3]; // for frustum culling
	s16 maxs[3];
	u16 firstmarksurface;
	u16 nummarksurfaces;
	byte ambient_level[NUM_AMBIENTS];
} dleaf_t;
typedef struct {
	s32 contents;
	s32 visofs; // -1 = no visibility info
	s16 mins[3]; // for frustum culling
	s16 maxs[3];
	u32 firstmarksurface;
	u32 nummarksurfaces;
	byte ambient_level[NUM_AMBIENTS];
} dl1leaf_t;
typedef struct {
	s32 contents;
	s32 visofs; // -1 = no visibility info
	f32 mins[3]; // for frustum culling
	f32 maxs[3];
	u32 firstmarksurface;
	u32 nummarksurfaces;
	byte ambient_level[NUM_AMBIENTS];
} dl2leaf_t;

typedef struct cvar_s cvar_t;                                            // cvar
typedef void (*cvarcallback_t)(cvar_t *);
struct cvar_s {
    const char      *name;
    const char      *string;
    unsigned int    flags;
    float           value;
    const char      *default_string; //johnfitz -- remember defaults for reset function
    cvarcallback_t  callback;
    cvar_t          *next;
};

// d*_t structures are on-disk representations
// m*_t structures are in-memory

typedef struct { f32 l, a, b; } lab_t;                                 // rgbtoi

typedef struct vrect_s {                                                  // vid
        s32 x,y,width,height; struct vrect_s *pnext;
} vrect_t;
typedef struct {
        u8  *buffer; // invisible buffer
        u8  *colormap; // 256 * VID_GRADES size
        u32 width;
        u32 height;
        f32 aspect; // width / height -- < 0 is taller than wide
        u32 numpages;
        u32 recalc_refdef; // if true, recalc vid-based stuff
} viddef_t;

typedef struct {                                                        // model
	vec3_t position;
} mvertex_t;
typedef struct texture_s {
	s8 name[16];
	unsigned width, height;
	unsigned shift; // Q64
	bool update_warp; //johnfitz -- update warp this frame
	struct msurface_s *texturechains[2]; // for texture chains
	s32 anim_total; // total tenths in sequence ( 0 = no)
	s32 anim_min, anim_max; // time for this frame min <=time< max
	struct texture_s *anim_next; // in the animation sequence
	struct texture_s *alternate_anims; // bmodels in frmae 1 use these
	unsigned offsets[MIPLEVELS]; // four mip maps stored
} texture_t;
typedef struct {
	f32 vecs[2][4];
	f32 mipadjust;
	texture_t *texture;
	s32 flags;
} mtexinfo_t;
typedef struct mplane_s {
	vec3_t normal;
	f32 dist;
	byte type; // for texture axis selection and fast side tests
	byte signbits; // signx + signy<<1 + signz<<1
	byte pad[2];
} mplane_t;
typedef struct msurface_s {
	s32 visframe; // should be drawn when node is crossed
	f32 mins[3]; // johnfitz -- for frustum culling
	f32 maxs[3]; // johnfitz -- for frustum culling
	mplane_t *plane;
	s32 flags;
	s32 firstedge; // look up in model->surfedges[], negative numbers
	s32 numedges; // are backwards edges
	s16 texturemins[2];
	s16 extents[2];
	s32 light_s, light_t; // gl lightmap coordinates
	struct msurface_s *texturechain;
	mtexinfo_t *texinfo;
	s32 vbo_firstvert; // index of surface's first vert in VBO lighting info
	s32 dlightframe;
	s32 dlightbits;
	s32 lightmaptexturenum;
	byte styles[MAXLIGHTMAPS];
	s32 cached_light[MAXLIGHTMAPS]; // values currently used in lightmap
	bool cached_dlight; // true if dynamic light in cache
	byte *samples; // [numstyles*surfsize]
	struct surfcache_s *cachespots[MIPLEVELS]; // surface generation data
} msurface_t;

typedef struct { // viewmodel lighting                                // r_local
	int ambientlight;
	int shadelight;
	float *plightvec;
} alight_t;
typedef struct bedge_s // clipped bmodel edges
{
	mvertex_t *v[2];
	struct bedge_s *pnext;
} bedge_t;
typedef struct {
	float fv[3]; // viewspace x, y
} auxvert_t;
typedef struct clipplane_s {
	vec3_t normal;
	float dist;
	struct clipplane_s *next;
	byte leftedge;
	byte rightedge;
	byte reserved[2];
} clipplane_t;
typedef struct btofpoly_s {
	int clipflags;
	msurface_t *psurf;
} btofpoly_t;
typedef enum {
	TEXTYPE_DEFAULT,
	TEXTYPE_CUTOUT,
	TEXTYPE_SKY,
	TEXTYPE_LAVA,
	TEXTYPE_SLIME,
	TEXTYPE_TELE,
	TEXTYPE_WATER,
	TEXTYPE_COUNT,
	TEXTYPE_FIRSTLIQUID = TEXTYPE_LAVA,
	TEXTYPE_LASTLIQUID = TEXTYPE_WATER,
	TEXTYPE_NUMLIQUIDS = TEXTYPE_LASTLIQUID + 1 - TEXTYPE_FIRSTLIQUID,
} textype_t;

typedef enum { ST_SYNC=0, ST_RAND } synctype_t;                      // modelgen
typedef enum { ALIAS_SINGLE=0, ALIAS_GROUP } aliasframetype_t;
typedef enum { ALIAS_SKIN_SINGLE=0, ALIAS_SKIN_GROUP } aliasskintype_t;
typedef struct {
	s32 ident;
	s32 version;
	vec3_t scale;
	vec3_t scale_origin;
	f32 boundingradius;
	vec3_t eyeposition;
	s32 numskins;
	s32 skinwidth;
	s32 skinheight;
	s32 numverts;
	s32 numtris;
	s32 numframes;
	synctype_t synctype;
	s32 flags;
	f32 size;
} mdl_t;
typedef struct {
	s32 onseam;
	s32 s;
	s32 t;
} stvert_t;
typedef struct dtriangle_s {
	s32 facesfront;
	s32 vertindex[3];
} dtriangle_t;
typedef struct { // This mirrors trivert_t in trilib.h, is present so Quake
	byte v[3]; // knows how to load this data
	byte lightnormalindex;
} trivertx_t;
typedef struct {
	trivertx_t bboxmin; // lightnormal isn't used
	trivertx_t bboxmax; // lightnormal isn't used
	s8 name[16]; // frame name from grabbing
} daliasframe_t;
typedef struct {
	s32 numframes;
	trivertx_t bboxmin; // lightnormal isn't used
	trivertx_t bboxmax; // lightnormal isn't used
} daliasgroup_t;
typedef struct {
	s32 numskins;
} daliasskingroup_t;
typedef struct {
	f32 interval;
} daliasinterval_t;
typedef struct {
	f32 interval;
} daliasskininterval_t;
typedef struct {
	aliasframetype_t type;
} daliasframetype_t;
typedef struct {
	aliasskintype_t type;
} daliasskintype_t;

typedef enum {SPR_SINGLE=0, SPR_GROUP, SPR_ANGLED}spriteframetype_t; // spritegn
typedef struct {
	s32 ident;
	s32 version;
	s32 type;
	f32 boundingradius;
	s32 width;
	s32 height;
	s32 numframes;
	f32 beamlength;
	synctype_t synctype;
} dsprite_t;
typedef struct {
	s32 origin[2];
	s32 width;
	s32 height;
} dspriteframe_t;
typedef struct { s32 numframes; } dspritegroup_t;
typedef struct { f32 interval; } dspriteinterval_t;
typedef struct { spriteframetype_t type; } dspriteframetype_t;

typedef struct cache_user_s                                              // zone
{
	void	*data;
} cache_user_t;

typedef struct {                                                     // protocol
	vec3_t          origin;
	vec3_t          angles;
	unsigned short  modelindex;     //johnfitz -- was int
	unsigned short  frame;          //johnfitz -- was int
	unsigned char   colormap;       //johnfitz -- was int
	unsigned char   skin;           //johnfitz -- was int
	unsigned char   alpha;          //johnfitz -- added
	unsigned char   scale;          //Quakespasm: for model scale support.
	int             effects;
} entity_state_t;
typedef struct {
	vec3_t  viewangles;
	float   forwardmove; // intended velocities
	float   sidemove;
	float   upmove;
} usercmd_t;

typedef struct entity_s {                                              // render
	bool forcelink; // model changed
	int update_type;
	entity_state_t baseline; // to fill in defaults in updates
	double msgtime; // time of last update
	vec3_t msg_origins[2]; // last two updates (0 is newest)
	vec3_t origin;
	vec3_t msg_angles[2]; // last two updates (0 is newest)
	vec3_t angles;
	struct model_s *model; // NULL = no model
	struct efrag_s *efrag; // linked list of efrags
	int frame;
	float syncbase; // for client-side animations
	byte *colormap;
	int effects; // light, particles, etc
	int skinnum; // for Alias models
	int visframe; // last frame this entity was found in an active leaf
	int dlightframe; // dynamic lighting
	int dlightbits;
	int trivial_accept;
	struct mnode_s *topnode; // for bmodels, first world node that splits
				 // bmodel, or NULL if not split
	byte alpha; //johnfitz -- alpha
	byte scale;
	byte lerpflags; //johnfitz -- lerping
	float lerpstart; //johnfitz -- animation lerping
	float lerptime; //johnfitz -- animation lerping
	float lerpfinish; //johnfitz -- lerping -- server sent us a more accurate interval, use it instead of 0.1
	short previouspose; //johnfitz -- animation lerping
	short currentpose; //johnfitz -- animation lerping
	short futurepose; //johnfitz -- animation lerping
	float movelerpstart; //johnfitz -- transform lerping
	vec3_t previousorigin; //johnfitz -- transform lerping
	vec3_t currentorigin; //johnfitz -- transform lerping
	vec3_t previousangles; //johnfitz -- transform lerping
} entity_t;
typedef struct efrag_s {
	struct mleaf_s *leaf;
	struct efrag_s *leafnext;
	struct entity_s *entity;
	struct efrag_s *entnext;
} efrag_t;
typedef struct {
	vrect_t vrect; // subwindow in video for refresh
		       // FIXME: not need vrect next field here?
	vrect_t aliasvrect; // scaled Alias version
	long vrectright, vrectbottom; // right & bottom screen coords
	long aliasvrectright, aliasvrectbottom; // scaled Alias versions
	float vrectrightedge; // rightmost right edge we care about,
			      // for use in edge list
	float fvrectx, fvrecty; // for floating-point compares
	float fvrectx_adj, fvrecty_adj; // left and top edges, for clamping
	long vrect_x_adj_shift20; //(vrect.x + 0.5 - epsilon) << 20
	long vrectright_adj_shift20; //(vrectright + 0.5 - epsilon) << 20
	float fvrectright_adj, fvrectbottom_adj; // right and bottom edges,
						 // for clamping
	float fvrectright; // rightmost edge, for Alias clamping
	float fvrectbottom; // bottommost edge, for Alias clamping
	float horizontalFieldOfView; // at Z = 1.0, this many X is visible 
				     // 2.0 = 90 degrees
	float xOrigin; // should probably allways be 0.5
	float yOrigin; // between be around 0.3 to 0.5

	vec3_t vieworg;
	vec3_t viewangles;

	float fov_x, fov_y;

	int ambientlight;
} refdef_t;

typedef struct {                                                        // model
	aliasskintype_t type;
	void *pcachespot;
	s32 skin;
} maliasskindesc_t;
typedef struct {
	s32 numskins;
	s32 intervals;
	maliasskindesc_t skindescs[1];
} maliasskingroup_t;
typedef enum { // ericw -- each texture has two chains, so we can clear the
	chain_world = 0, // model chains without affecting the world
	chain_model = 1
} texchain_t;
typedef struct {
	u32 v[2];
	u32 cachededgeoffset;
} medge_t;
typedef struct mnode_s {
	s32 contents; // common with leaf. 0, to differentiate from leafs
	s32 visframe; // node needs to be traversed if current
	f32 minmaxs[6]; // for bounding box culling
	struct mnode_s *parent;
	mplane_t *plane; // node specific
	struct mnode_s *children[2];
	u32 firstsurface;
	u32 numsurfaces;
} mnode_t;
typedef struct mleaf_s {
	s32 contents; // common with node. Will be a negative contents number
	s32 visframe; // node needs to be traversed if current
	f32 minmaxs[6]; // for bounding box culling
	struct mnode_s *parent;
	byte *compressed_vis; // leaf specific
	efrag_t *efrags;
	msurface_t **firstmarksurface;
	s32 nummarksurfaces;
	s32 key; // BSP sequence number for leaf's contents
	byte ambient_sound_level[NUM_AMBIENTS];
} mleaf_t;
typedef struct mclipnode_s { //johnfitz -- for clipnodes>32k
	s32 planenum;
	s32 children[2]; // negative numbers are contents
} mclipnode_t;
typedef struct {
	s32 planenum;
	s16 children[2]; // negative numbers are contents
} dsclipnode_t;
typedef struct {
	s32 planenum;
	s32 children[2]; // negative numbers are contents
} dlclipnode_t;
typedef struct {
	mclipnode_t *clipnodes; //johnfitz -- was dclipnode_t
	mplane_t *planes;
	s32 firstclipnode;
	s32 lastclipnode;
	vec3_t clip_mins;
	vec3_t clip_maxs;
} hull_t;
typedef struct mspriteframe_s {
	s32 width, height;
	f32 up, down, left, right;
	f32 smax, tmax; //johnfitz -- image might be padded
	byte pixels[4];
} mspriteframe_t;
typedef struct {
	s32 numframes;
	f32 *intervals;
	mspriteframe_t *frames[1];
} mspritegroup_t;
typedef struct {
	spriteframetype_t type;
	mspriteframe_t *frameptr;
} mspriteframedesc_t;
typedef struct {
	s32 type;
	s32 maxwidth;
	s32 maxheight;
	s32 numframes;
	f32 beamlength;
	void *cachespot;
	mspriteframedesc_t frames[1];
} msprite_t;
// Alias models are position independent, so the cache manager can move them.
typedef struct aliasmesh_s{//from RMQEngine, split out to keep vertex sizes down
	f32 st[2];
	u16 vertindex;
} aliasmesh_t;
typedef struct meshxyz_s {
	byte xyz[4];
	s8 normal[4];
} meshxyz_t;
typedef struct meshst_s {
	f32 st[2];
} meshst_t; // RMQEngine end
typedef struct {
	aliasframetype_t type;
	s32 firstpose;
	s32 numposes;
	f32 interval;
	trivertx_t bboxmin;
	trivertx_t bboxmax;
	s32 frame;
	s8 name[16];
} maliasframedesc_t;
typedef struct {
	trivertx_t bboxmin;
	trivertx_t bboxmax;
	s32 frame;
} maliasgroupframedesc_t;
typedef struct {
	s32 numframes;
	s32 intervals;
	maliasgroupframedesc_t frames[1];
} maliasgroup_t;
typedef struct mtriangle_s {
	s32 facesfront;
	s32 vertindex[3];
} mtriangle_t;
typedef struct {
	s32 ident;
	s32 version;
	vec3_t scale;
	vec3_t scale_origin;
	f32 boundingradius;
	vec3_t eyeposition;
	s32 numskins;
	s32 skinwidth;
	s32 skinheight;
	s32 numverts;
	s32 numtris;
	s32 numframes;
	synctype_t synctype;
	s32 flags;
	f32 size;
	//ericw -- used to populate vbo
	s32 numverts_vbo; // number of verts with unique x,y,z,s,t
	intptr_t meshdesc; // offset into extradata: numverts_vbo aliasmesh_t
	s32 numindexes;
	intptr_t indexes; // offset into extradata: numindexes unsigned shorts
	intptr_t vertexes; // offset into extradata: numposes*vertsperframe trivertx_t
	s32 numposes;
	s32 poseverts;
	s32 posedata; // numposes*poseverts trivert_t
	s32 commands; // gl command list with embedded s/t
	s32 texels[MAX_SKINS]; // only for player skins
	s32 model;
	s32 stverts;
	s32 skindesc;
	s32 triangles;
	maliasframedesc_t frames[1]; // variable sized
} aliashdr_t;
typedef enum {mod_brush, mod_sprite, mod_alias} modtype_t;
typedef struct model_s {
	f32 radius;
	s8 name[MAX_QPATH];
	u32 path_id; // path id of game dir that this model came from
	bool needload; // bmodels and sprites don't cache normally
	modtype_t type;
	s32 numframes;
	synctype_t synctype;
	s32 flags;
	vec3_t mins, maxs; // volume occupied by the model graphics
	vec3_t ymins, ymaxs; //johnfitz -- bounds for entities with nonzero yaw
	vec3_t rmins, rmaxs; //johnfitz -- bounds for entities with nonzero pitch or roll
	bool clipbox; // solid volume for clipping
	vec3_t clipmins, clipmaxs;
	s32 firstmodelsurface, nummodelsurfaces; // brush model
	s32 numsubmodels;
	dmodel_t *submodels;
	s32 numplanes;
	mplane_t *planes;
	s32 numleafs; // number of visible leafs, not counting 0
	mleaf_t *leafs;
	s32 numvertexes;
	mvertex_t *vertexes;
	s32 numedges;
	medge_t *edges;
	s32 numnodes;
	mnode_t *nodes;
	s32 numtexinfo;
	mtexinfo_t *texinfo;
	s32 numsurfaces;
	msurface_t *surfaces;
	s32 numsurfedges;
	s32 *surfedges;
	s32 numclipnodes;
	mclipnode_t *clipnodes; //johnfitz -- was dclipnode_t
	s32 nummarksurfaces;
	msurface_t **marksurfaces;
	hull_t hulls[MAX_MAP_HULLS];
	s32 numtextures;
	texture_t **textures;
	byte *visdata;
	byte *lightdata;
	s8 *entities;
	bool viswarn; // for Mod_DecompressVis()
	s32 bspversion;
	s32 contentstransparent; //spike -- added this so we can disable glitchy
				 //wateralpha where its not supported.
	bool haslitwater;
	// alias model
	s32 vboindexofs; // offset in vbo of the hdr->numindexes unsigned shorts
	s32 vboxyzofs; // offset of hdr->numposes*hdr->numverts_vbo meshxyz_t
	s32 vbostofs; // offset in vbo of hdr->numverts_vbo meshst_t
	cache_user_t cache;//extra model data, only access through Mod_Extradata
} model_t;

typedef struct espan_s {                                             // r_shared
	long u, v, count;
	struct espan_s *pnext;
} espan_t;
typedef struct surf_s {
	struct surf_s *next; // active surface stack in r_edge.c
	struct surf_s *prev; // used in r_edge.c for active surf stack
	struct espan_s *spans; // pointer to linked list of spans to draw
	int key; // sorting key (BSP order)
	long last_u; // set during tracing
	int spanstate; // 0 = not in span
		       // 1 = in span
		       // -1 = in inverted span (end before start)
	int flags; // currentface flags
	void *data; // associated data like msurface_t
	entity_t *entity;
	float nearzi; // nearest 1/z on surface, for mipmapping
	bool insubmodel;
	float d_ziorigin, d_zistepu, d_zistepv;
	int pad[2]; // to 64 bytes
} surf_t;
typedef struct edge_s {
	long u;
	long u_step;
	struct edge_s *prev, *next;
	unsigned short surfs[2];
	struct edge_s *nextremove;
	float nearzi;
	medge_t *owner;
} edge_t;

typedef struct {                                                      // d_iface
	float u, v;
	float s, t;
	float zi;
} emitpoint_t;
typedef enum {
	pt_static, pt_grav, pt_slowgrav, pt_fire, pt_explode, pt_explode2, pt_blob, pt_blob2
} ptype_t;
typedef struct particle_s {
	// driver-usable fields
	vec3_t org;
	float color;
	// drivers never touch the following fields
	struct particle_s *next;
	vec3_t vel;
	float ramp;
	float die;
	ptype_t type;
} particle_t;
typedef struct polyvert_s {
	float u, v, zi, s, t;
} polyvert_t;
typedef struct polydesc_s {
	int numverts;
	float nearzi;
	msurface_t *pcurrentface;
	polyvert_t *pverts;
} polydesc_t;
typedef struct finalvert_s {
	int v[6]; // u, v, s, t, l, 1/z
	int flags;
	float reserved;
} finalvert_t;
typedef struct {
	void *pskin;
	maliasskindesc_t *pskindesc;
	int skinwidth;
	int skinheight;
	mtriangle_t *ptriangles;
	finalvert_t *pfinalverts;
	int numtriangles;
	int drawtype;
	int seamfixupX16;
} affinetridesc_t;
typedef struct {
	float u, v, zi, color;
} screenpart_t;
typedef struct {
	int nump;
	emitpoint_t *pverts; // there's room for an extra element at [nump], if the driver wants to duplicate element [0] at element [nump] to avoid dealing with wrapping
	mspriteframe_t *pspriteframe;
	vec3_t vup, vright, vpn; // in worldspace
	float nearzi;
} spritedesc_t;
typedef struct {
	int u, v;
	float zi;
	int color;
} zpointdesc_t;
typedef struct {
	pixel_t *surfdat; // destination for generated surface
	int rowbytes; // destination logical width in bytes
	msurface_t *surf; // description for surface to generate
	fixed8_t lightadj[MAXLIGHTMAPS];
	// adjust for lightmap levels for dynamic lighting
	texture_t *texture; // corrected for animating textures
	int surfmip; // mipmapped ratio of surface texels / world pixels
	int surfwidth; // in mipmapped texels
	int surfheight; // in mipmapped texels
} drawsurf_t;
#endif
