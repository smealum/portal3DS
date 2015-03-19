#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>

#include "game/room.h"
#include "gfx/gs.h"

#include "physics/physics.h"

#include "room_vsh_shbin.h"

DVLB_s* roomDvlb;
shaderProgram_s roomProgram;
const u32 roomBaseAddr=0x14000000;

int roomUniformTextureDimensions;

void roomInit()
{
	roomDvlb = DVLB_ParseFile((u32*)room_vsh_shbin, room_vsh_shbin_size);

	if(!roomDvlb)return;

	shaderProgramInit(&roomProgram);
	shaderProgramSetVsh(&roomProgram, &roomDvlb->DVLE[0]);

	roomUniformTextureDimensions = shaderInstanceGetUniformLocation(roomProgram.vertexShader, "textureDimensions");
}

void roomExit()
{
	shaderProgramFree(&roomProgram);
	DVLB_Free(roomDvlb);
}

void initRectangleList(rectangleList_s* p)
{
	p->first=NULL;
	p->num=0;
}

vect3Di_s orientVector(vect3Di_s v, u8 k)
{
	vect3Di_s u;

	switch(k)
	{
		case 0:
			u.x=-v.z;
			u.y=v.y;
			u.z=v.x;
			break;
		case 1:
			u.x=v.z;
			u.y=v.y;
			u.z=-v.x;
			break;
		case 4:
			u.x=v.x;
			u.y=v.y;
			u.z=-v.z;
			break;
		default:
			u=v;
			break;
	}

	return u;
}

void invertRectangle(rectangle_s* rec)
{
	if(!rec)return;

	if(rec->size.x)
	{
		rec->position.x+=rec->size.x;
		rec->size.x=-rec->size.x;
	}else{
		rec->position.z+=rec->size.z;
		rec->size.z=-rec->size.z;
	}
}

void roomOriginSize(room_s* r, vect3Di_s* o, vect3Di_s* s)
{
	if(!r || (!o && !s))return;

	vect3Di_s m=vect3Di(8192,8192,8192); vect3Di_s M=vect3Di(0,0,0);

	listCell_s *lc=r->rectangles.first;

	while(lc)
	{
		m=vmini(lc->data.position,m);
		m=vmini(vaddi(lc->data.position,lc->data.size),m);
		M=vmaxi(lc->data.position,M);
		M=vmaxi(vaddi(lc->data.position,lc->data.size),M);
		lc=lc->next;
	}

	if(o)*o=m;
	if(s)*s=vsubi(M,m);
}


void insertRoom(room_s* r1, room_s* r2, vect3Di_s v, u8 orientation)
{
	if(!r1 || !r2)return;

	listCell_s *lc=r2->rectangles.first;

	vect3Di_s o=vect3Di(0,0,0), s=vect3Di(0,0,0);
	roomOriginSize(r2,&o,&s);

	switch(orientation) //TODO : do pre-rotation ?
	{
		case 0:
			v.z-=s.x/2; //not a mistake
			break;
		case 1:
			v.z+=s.x/2; //not a mistake
			break;
		case 4:	case 5:
			v.x-=s.x/2;
			break;
	}
	v.y-=4;

	while(lc)
	{
		rectangle_s rec=lc->data;
		rec.position=vsubi(rec.position,o);

		//rotate
		rec.position=orientVector(rec.position,orientation);
		rec.size=orientVector(rec.size,orientation);
		if(!(orientation%2) || orientation==1)invertRectangle(&rec);

		rec.position=vaddi(rec.position,v);
		rectangle_s* recp=addRoomRectangle(r1, rec);
		if(recp)
		{
			// recp->hide=true; //TEMP ?
			recp->collides=!lc->data.portalable;
			// recp->lightData.vertex=lc->data.lightData.vertex;
		}
		lc=lc->next;
	}
}

rectangle_s* addRectangle(rectangle_s r, rectangleList_s* p)
{
	listCell_s* pc=(listCell_s*)malloc(sizeof(listCell_s));
	pc->next=p->first;
	pc->data=r;
	p->first=pc;
	p->num++;
	return &pc->data;
}

void popRectangle(rectangleList_s* p)
{
	p->num--;
	if(p->first)
	{
		listCell_s* pc=p->first;
		p->first=pc->next;
		free(pc);
	}
}

rectangle_s* addRoomRectangle(room_s* r, rectangle_s rec)
{
	if((!rec.size.x && (!rec.size.z || !rec.size.y)) || (!rec.size.y && !rec.size.z))return NULL;
	rec.hide=false;
	rec.aar=NULL;
	return addRectangle(rec, &r->rectangles);
}

void removeRoomRectangles(room_s* r)
{
	while(r->rectangles.num)popRectangle(&r->rectangles);
}

void initRoomGrid(room_s* r)
{
	if(!r || !r->width || !r->height)return;
	
	listCell_s *lc=r->rectangles.first;
	while(lc)
	{
		if(lc->data.position.x < r->rectangleGridOrigin.x)r->rectangleGridOrigin.x=lc->data.position.x;
		else if(lc->data.position.x > r->rectangleGridSize.x)r->rectangleGridSize.x=lc->data.position.x;
		if(lc->data.position.z < r->rectangleGridOrigin.z)r->rectangleGridOrigin.z=lc->data.position.z;
		else if(lc->data.position.z > r->rectangleGridSize.z)r->rectangleGridSize.z=lc->data.position.z;
		lc=lc->next;
	}

	r->rectangleGridSize.x-=r->rectangleGridOrigin.x;
	r->rectangleGridSize.z-=r->rectangleGridOrigin.z;

	r->rectangleGridSize.x=r->rectangleGridSize.x/CELLSIZE+1;
	r->rectangleGridSize.z=r->rectangleGridSize.z/CELLSIZE+1;

	printf("%d %d\n", r->rectangleGridSize.x, r->rectangleGridSize.z);
	
	r->rectangleGrid=malloc(sizeof(gridCell_s)*r->rectangleGridSize.x*r->rectangleGridSize.z);
	
	int i;
	for(i=0;i<r->rectangleGridSize.x*r->rectangleGridSize.z;i++)
	{
		r->rectangleGrid[i].rectangles=NULL;
		r->rectangleGrid[i].numRectangles=0;
	}
}

void initRoom(room_s* r, u16 w, u16 h, vect3Df_s p)
{
	if(!r)return;
	
	r->width=w;
	r->height=h;
	r->position=p;
	
	initRectangleList(&r->rectangles);

	r->vertexBuffer=NULL;
	r->indexBuffers=NULL;
	
	// if(r->height && r->width)
	// {
	// 	r->materials=malloc(r->height*r->width*sizeof(material_s*));
	// 	int i;for(i=0;i<r->height*r->width;i++){r->materials[i]=NULL;}
	// }else r->materials=NULL;

	r->rectangleGrid=NULL;
}

gridCell_s* getCurrentCell(room_s* r, vect3Df_s o)
{
	if(!r)return NULL;
	
	o=vsubf(o,convertRectangleVector(vect3Di(r->rectangleGridOrigin.x,0,r->rectangleGridOrigin.y)));
	o=vdivf(o,CELLSIZE*TILESIZE*2);

	int x = (int)o.x;
	int z = (int)o.z;
	
	if(x>=0 && x<r->rectangleGridSize.x && z>=0 && z<r->rectangleGridSize.z)return &r->rectangleGrid[x+z*r->rectangleGridSize.x];
	return NULL;
}

void generateGridCell(room_s* r, gridCell_s* gc, u16 x, u16 y)
{
	if(!r || !gc)return;
	
	if(gc->rectangles)free(gc->rectangles);
	gc->numRectangles=0;
	
	x+=r->rectangleGridOrigin.x;y+=r->rectangleGridOrigin.z; //offset
	x*=CELLSIZE*2;y*=CELLSIZE*2; //so getting the center isn't a problem
	x+=CELLSIZE;y+=CELLSIZE; //center
	
	listCell_s *lc=r->rectangles.first;
	while(lc)
	{
		if((abs(x-(lc->data.position.x*2+lc->data.size.x))<=(CELLSIZE+abs(lc->data.size.x))) && (abs(y-(lc->data.position.z*2+lc->data.size.z))<=(CELLSIZE+abs(lc->data.size.z))))
		{
			gc->numRectangles++;
		}
		lc=lc->next;
	}
	gc->rectangles=malloc(sizeof(rectangle_s*)*gc->numRectangles);
	gc->numRectangles=0;
	lc=r->rectangles.first;
	while(lc)
	{
		if((abs(x-(lc->data.position.x*2+lc->data.size.x))<=(CELLSIZE+abs(lc->data.size.x))) && (abs(y-(lc->data.position.z*2+lc->data.size.z))<=(CELLSIZE+abs(lc->data.size.z))))
		{
			gc->rectangles[gc->numRectangles++]=&lc->data;
		}
		lc=lc->next;
	}
}

void generateRoomGrid(room_s* r)
{
	if(!r)return;

	initRoomGrid(r);
	
	int i, j;
	for(i=0;i<r->rectangleGridSize.x;i++)
	{
		for(j=0;j<r->rectangleGridSize.z;j++)
		{
			generateGridCell(r,&r->rectangleGrid[i+j*r->rectangleGridSize.x],i,j);
		}
	}
}

void transferRoomRectangles(room_s* r)
{
	if(!r)return;
	listCell_s *lc=r->rectangles.first;
	int i=0;
	while(lc)
	{
		// lc->data.AARid=
		physicsCreateAar(&lc->data.aar, convertRectangleVector(lc->data.position), convertRectangleVector(lc->data.size), vmulf(lc->data.normal, -1.0f));
		lc=lc->next;
		i++;
	}
}

u8 getNormalOrientation(vect3Di_s v)
{
	if(v.x>0)return 0;
	if(v.x<0)return 1;
	if(v.y>0)return 2;
	if(v.y<0)return 3;
	if(v.z>0)return 4;
	if(v.z<0)return 5;
	return 0;
}

void getRectangleUnitVectors(rectangle_s* rec, vect3Di_s* v1, vect3Di_s* v2)
{
	if(!rec)return;

	if(rec->size.x)
	{
		if(v1)*v1=vect3Di(rec->size.x, 0, 0);
		if(v2)
		{
			if(rec->size.y)*v2=vect3Di(0, rec->size.y, 0);
			else *v2=vect3Di(0, 0, rec->size.z);
		}
	}else{
		if(v1)*v1=vect3Di(0, rec->size.y, 0);
		if(v2)*v2=vect3Di(0, 0, rec->size.z);
	}
}

vect3Df_s convertRectangleVector(vect3Di_s v)
{
	return vect3Df(v.x*TILESIZE_FLOAT*2, v.y*HEIGHTUNIT_FLOAT, v.z*TILESIZE_FLOAT*2);
}

bool collideLineRectangle(rectangle_s* rec, vect3Df_s o, vect3Df_s v, float d, float* kk, vect3Df_s* ip)
{
	if(!rec)return false;
	vect3Df_s n=vect3Df(fabs(rec->normal.x),fabs(rec->normal.y),fabs(rec->normal.z));
	float p1=vdotf(v,n);
	if(fabs(p1)>0.001f)
	{
		vect3Df_s p = convertRectangleVector(rec->position);
		vect3Df_s s = convertRectangleVector(rec->size);
		
		float p2=vdotf(vsubf(p,o),n);

		float k=p2/p1;
		s8 sign=((s.x>0)^(s.y<0)^(s.z>0)^(p1<0))?(-1):(1);
		if(kk)
		{
			*kk=k+sign;
		}
		if(k<0 || k>d){return false;}
		vect3Df_s i=vaddf(o,vmulf(v,k));
		if(ip)*ip=i;
		i=vsubf(i,p);
		
		bool r=true;
		if(s.x)
		{
			if(s.x>0)r=r&&i.x<s.x&&i.x>=0;
			else r=r&&i.x>s.x&&i.x<=0;
		}
		if(s.y)
		{
			if(s.y>0)r=r&&i.y<s.y&&i.y>=0;
			else r=r&&i.y>s.y&&i.y<=0;
		}
		if(s.z)
		{
			if(s.z>0)r=r&&i.z<s.z&&i.z>=0;
			else r=r&&i.z>s.z&&i.z<=0;
		}
		return r;
	}
	return false;
}

rectangle_s* collideGridCell(gridCell_s* gc, rectangle_s* rec, vect3Df_s l, vect3Df_s u, float d, vect3Df_s* i, vect3Df_s* n)
{
	if(!gc)return NULL;
	vect3Df_s v;
	int j;
	for(j=0;j<gc->numRectangles;j++)
	{
		if(gc->rectangles[j]!=rec && gc->rectangles[j]->collides)
		{
			if(collideLineRectangle(gc->rectangles[j],l,u,d,NULL,&v))
			{
				if(i)*i=v;
				if(n)*n=gc->rectangles[j]->normal;
				return gc->rectangles[j];
			}
		}
	}
	return NULL;
}

rectangle_s* collideLineMapClosest(room_s* r, rectangle_s* rec, vect3Df_s l, vect3Df_s u, float d, vect3Df_s* i, float* lk)
{
	if(!r)return NULL;
	listCell_s *lc=r->rectangles.first;
	vect3Df_s v;
	float lowestK=d;
	rectangle_s* hit=NULL;
	while(lc)
	{
		if(&lc->data!=rec && lc->data.collides)
		{
			float k;
			if(collideLineRectangle(&lc->data,l,u,lowestK,&k,&v))
			{
				if(k<lowestK)
				{
					if(i)*i=v;
					if(lk)*lk=k;
					lowestK=k;
					hit=&lc->data;
				}
			}
		}
		lc=lc->next;
	}
	return hit;
}

int findTexture(texture_s* t, texture_s** tb, int n)
{
	if(!t || !tb)return -1;
	int i;
	for(i=0;i<n;i++)if(t==tb[i])return i;
	return -1;
}

void generateRectangleGeometry(rectangle_s* rec, vect3Di_s* texCoords, rectangleVertex_s* vbuf, int* numvert, u16* ibuf, int* numind)
{
	if(!rec || !texCoords || !vbuf || !ibuf || !numvert || !numind)return;

	vect3Di_s v1, v2;
	getRectangleUnitVectors(rec, &v1, &v2);

	int vertexIndices[4];
	vect3Di_s vertices[4] = {rec->position, vaddi(rec->position, v1), vaddi(rec->position, rec->size), vaddi(rec->position, v2)};
	
	// printf("%f %f, %f %f\n", (texCoords[0].x)*0.0078125, (texCoords[0].y)*0.0078125, (texCoords[2].x)*0.0078125, (texCoords[2].y)*0.0078125);

	int i;
	for(i=0; i<4; i++)
	{
		vertexIndices[i] = (*numvert)++;
		vbuf[vertexIndices[i]] = (rectangleVertex_s){vertices[i].x, vertices[i].y, vertices[i].z, (texCoords[i].x), (texCoords[i].y), 0, 0};
	}

	if(rec->lightData.lightMap)
	{
		if(rec->lightData.lightMap->rot)
		{
			vect3Di_s p1 = rec->lightData.lightMap->lmPos;
			vect3Di_s p2 = vaddi(rec->lightData.lightMap->lmPos, vect3Di(rec->lightData.lightMap->lmSize.y, rec->lightData.lightMap->lmSize.x, 0));

			printf("%d %d : %d %d\n",p1.x,p1.y,p2.x,p2.y);

			vbuf[vertexIndices[0]].u2 = p1.x;
			vbuf[vertexIndices[0]].v2 = p1.y;

			vbuf[vertexIndices[1]].u2 = p1.x;
			vbuf[vertexIndices[1]].v2 = p2.y - 1;

			vbuf[vertexIndices[2]].u2 = p2.x - 1;
			vbuf[vertexIndices[2]].v2 = p2.y - 1;

			vbuf[vertexIndices[3]].u2 = p2.x - 1;
			vbuf[vertexIndices[3]].v2 = p1.y;
		}else{
			vect3Di_s p1 = rec->lightData.lightMap->lmPos;
			vect3Di_s p2 = vaddi(rec->lightData.lightMap->lmPos, rec->lightData.lightMap->lmSize);

			vbuf[vertexIndices[0]].u2 = p1.x;
			vbuf[vertexIndices[0]].v2 = p1.y;

			vbuf[vertexIndices[1]].u2 = p2.x - 1;
			vbuf[vertexIndices[1]].v2 = p1.y;

			vbuf[vertexIndices[2]].u2 = p2.x - 1;
			vbuf[vertexIndices[2]].v2 = p2.y - 1;

			vbuf[vertexIndices[3]].u2 = p1.x;
			vbuf[vertexIndices[3]].v2 = p2.y - 1;
		}
	}

	ibuf[(*numind)++] = vertexIndices[0];
	ibuf[(*numind)++] = vertexIndices[1];
	ibuf[(*numind)++] = vertexIndices[3];

	ibuf[(*numind)++] = vertexIndices[1];
	ibuf[(*numind)++] = vertexIndices[2];
	ibuf[(*numind)++] = vertexIndices[3];
}

void generateRoomGeometry(room_s* r)
{
	if(!r)return;

	if(r->vertexBuffer)linearFree(r->vertexBuffer);
	if(r->numIndices)linearFree(r->numIndices);
	if(r->indexBufferTextures)linearFree(r->indexBufferTextures);
	if(r->indexBuffers)
	{
		int i; for(i=0; i<r->numIndexBuffers; i++)linearFree(r->indexBuffers[i]);
		free(r->indexBuffers);
	}
	r->vertexBuffer=NULL;
	r->numIndices=NULL;
	r->indexBufferTextures=NULL;
	r->indexBuffers=NULL;
	r->numIndexBuffers=0;

	rectangleVertex_s* tmpVertex=malloc(sizeof(rectangleVertex_s)*r->rectangles.num*4);
	if(!tmpVertex)return;

	{
		texture_s** tmpTextures=calloc(TEXTURES_NUM, sizeof(texture_s*));
		int* tmpNumIndices=calloc(TEXTURES_NUM, sizeof(int));

		r->numIndexBuffers=0;
		listCell_s* l=r->rectangles.first;
		while(l)
		{
			if(l->data.hide){l=l->next; continue;}
			texture_s* t=getRectangleTexture(&l->data);
			int i=findTexture(t, tmpTextures, r->numIndexBuffers);
			if(i<0)tmpTextures[i=(r->numIndexBuffers++)]=t;
			tmpNumIndices[i]++;
			l=l->next;
		}
		r->indexBuffers=calloc(r->numIndexBuffers, sizeof(u16*));
		r->numIndices=calloc(r->numIndexBuffers, sizeof(int));
		r->indexBufferTextures=calloc(r->numIndexBuffers, sizeof(texture_s*));
		int i; for(i=0; i<r->numIndexBuffers; i++)r->indexBuffers[i]=linearAlloc(sizeof(u16)*6*tmpNumIndices[i]);

		free(tmpTextures);
		free(tmpNumIndices);
	}

	{
		r->numIndexBuffers=0;

		r->vertexBuffer=tmpVertex;
		r->numVertices=0;
		listCell_s* l=r->rectangles.first;
		while(l)
		{
			if(l->data.hide){l=l->next; continue;}

			texture_s* t=getRectangleTexture(&l->data);
			int b=findTexture(t, r->indexBufferTextures, r->numIndexBuffers);
			if(b<0)
			{
				r->indexBufferTextures[b=r->numIndexBuffers++]=t;
			}

			vect3Di_s texCoords[4];
			getMaterialTextureCoord(&l->data, texCoords);
			generateRectangleGeometry(&l->data, texCoords, r->vertexBuffer, &r->numVertices, r->indexBuffers[b], &r->numIndices[b]);

			l=l->next;
		}
	}

	r->vertexBuffer=linearAlloc(sizeof(rectangleVertex_s)*r->numVertices);
	memcpy(r->vertexBuffer, tmpVertex, sizeof(rectangleVertex_s)*r->numVertices);

	free(tmpVertex);
}

void drawRoom(room_s* r)
{
	if(!r)return;

	gsSwitchRenderMode(-1);

	gsSetShader(&roomProgram);

	GPU_SetAttributeBuffers(
		3, // number of attributes
		(u32*)osConvertVirtToPhys(roomBaseAddr), // we use the start of linear heap as base since that's where all our buffers are located
		GPU_ATTRIBFMT(0, 3, GPU_SHORT)|GPU_ATTRIBFMT(1, 2, GPU_SHORT)|GPU_ATTRIBFMT(2, 2, GPU_SHORT), // we want v0, v1 and v2
		0xFF8, // mask : we want v0, v1 and v2
		0x210, // permutation : we use identity
		1, // number of buffers : we have one attribute per buffer
		(u32[]){(u32)r->vertexBuffer-roomBaseAddr}, // buffer offsets (placeholders)
		(u64[]){0x210}, // attribute permutations for each buffer
		(u8[]){3} // number of attributes for each buffer
		);

	GPU_SetTextureEnable(GPU_TEXUNIT0|GPU_TEXUNIT1);
	
	GPU_SetTexEnv(0, 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_TEXTURE1, GPU_PRIMARY_COLOR),
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
		GPU_TEVOPERANDS(0,2,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_MODULATE, GPU_MODULATE, 
		0xFFFFFFFF);

	gsPushMatrix();

		gsScale(TILESIZE_FLOAT*2, HEIGHTUNIT_FLOAT, TILESIZE_FLOAT*2);

		gsUpdateTransformation();

		int i;
		for(i=0; i<r->numIndexBuffers; i++)
		{
			textureBind(r->indexBufferTextures[i], GPU_TEXUNIT0);
			textureBind(r->lightingData.data.lightMap.texture, GPU_TEXUNIT1);
			GPU_SetFloatUniform(GPU_VERTEX_SHADER, roomUniformTextureDimensions, (u32*)(float[]){0.0f, 0.0f, 1.0f / r->indexBufferTextures[i]->height, 1.0f / r->indexBufferTextures[i]->width}, 1);
			GPU_DrawElements(GPU_UNKPRIM, (u32*)((u32)r->indexBuffers[i]-roomBaseAddr), r->numIndices[i]);
		}

	GPU_SetTextureEnable(GPU_TEXUNIT0);

	GPU_SetTexEnv(0, 
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
		GPU_TEVSOURCES(GPU_TEXTURE0, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR),
		GPU_TEVOPERANDS(0,0,0), 
		GPU_TEVOPERANDS(0,0,0), 
		GPU_MODULATE, GPU_MODULATE, 
		0xFFFFFFFF);

	gsPopMatrix();
}
