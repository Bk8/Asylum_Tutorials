//*************************************************************************************************************
#include <d3dx9.h>
#include <iostream>
#include <string>

#include "../common/dxext.h"

#define HELP_TEXT	\
	"1 - Sobel filter\n" \
	"2 - Stencil contours\n"

// helper macros
#define TITLE				"Shader sample 37: Cel shading"
#define MYERROR(x)			{ std::cout << "* Error: " << x << "!\n"; }
#define MYVALID(x)			{ if( FAILED(hr = x) ) { MYERROR(#x); return hr; } }
#define SAFE_RELEASE(x)		{ if( (x) ) { (x)->Release(); (x) = NULL; } }

// external variables
extern long screenwidth;
extern long screenheight;
extern LPDIRECT3DDEVICE9 device;
extern HWND hwnd;

// sample variables
LPD3DXMESH						mesh			= NULL;
LPD3DXEFFECT					effect			= NULL;
LPDIRECT3DTEXTURE9				texture			= NULL;
LPDIRECT3DTEXTURE9				intensity		= NULL;
LPDIRECT3DTEXTURE9				colortarget		= NULL;
LPDIRECT3DTEXTURE9				normaltarget	= NULL;
LPDIRECT3DTEXTURE9				edgetarget		= NULL;
LPDIRECT3DTEXTURE9				text			= NULL;
LPDIRECT3DSURFACE9				colorsurface	= NULL;
LPDIRECT3DSURFACE9				normalsurface	= NULL;
LPDIRECT3DSURFACE9				edgesurface		= NULL;
LPDIRECT3DVERTEXDECLARATION9	vertexdecl		= NULL;
D3DXMATRIX						world, view, proj;
bool							useedgedetect	= true;

float quadvertices[36] =
{
	-0.5f, -0.5f, 0, 1,		0, 0,
	-0.5f, -0.5f, 0, 1,		1, 0,
	-0.5f, -0.5f, 0, 1,		0, 1,

	-0.5f, -0.5f, 0, 1,		0, 1,
	-0.5f, -0.5f, 0, 1,		1, 0,
	-0.5f, -0.5f, 0, 1,		1, 1
};

float textvertices[36] =
{
	9.5f,			9.5f,	0, 1,	0, 0,
	521.5f,			9.5f,	0, 1,	1, 0,
	9.5f,	512.0f + 9.5f,	0, 1,	0, 1,

	9.5f,	512.0f + 9.5f,	0, 1,	0, 1,
	521.5f,			9.5f,	0, 1,	1, 0,
	521.5f,	512.0f + 9.5f,	0, 1,	1, 1
};

static HRESULT CreateColorTex(LPDIRECT3DDEVICE9 device, DWORD color, LPDIRECT3DTEXTURE9* texture)
{
	UINT width = 4;
	UINT height = 4;

	D3DLOCKED_RECT rect;
	HRESULT hr = device->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, texture, NULL);

	if( FAILED(hr) )
		return hr;

	(*texture)->LockRect(0, &rect, NULL, 0);

	for( UINT i = 0; i < height; ++i )
	{
		for( UINT j = 0; j < width; ++j )
		{
			DWORD* ptr = ((DWORD*)rect.pBits + i * width + j);
			*ptr = color;
		}
	}

	(*texture)->UnlockRect(0);
	return S_OK;
}

HRESULT InitScene()
{
	HRESULT hr;
	LPD3DXBUFFER errors = NULL;

	D3DVERTEXELEMENT9 decl[] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITIONT, 0 },
		{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	SetWindowText(hwnd, TITLE);

	quadvertices[6] = quadvertices[24] = quadvertices[30]	= (float)screenwidth - 0.5f;
	quadvertices[13] = quadvertices[19] = quadvertices[31]	= (float)screenheight - 0.5f;

	MYVALID(CreateColorTex(device, 0xff77FF70, &texture));
	MYVALID(D3DXLoadMeshFromXA("../media/meshes/knot.X", D3DXMESH_MANAGED, device, NULL, NULL, NULL, NULL, &mesh));
	MYVALID(D3DXCreateTextureFromFileA(device, "../media/textures/intensity.png", &intensity));

	MYVALID(device->CreateTexture(screenwidth, screenheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &colortarget, NULL));
	MYVALID(device->CreateTexture(screenwidth, screenheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &normaltarget, NULL));
	MYVALID(device->CreateTexture(screenwidth, screenheight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &edgetarget, NULL));
	MYVALID(device->CreateVertexDeclaration(decl, &vertexdecl));

	edgetarget->GetSurfaceLevel(0, &edgesurface);
	colortarget->GetSurfaceLevel(0, &colorsurface);
	normaltarget->GetSurfaceLevel(0, &normalsurface);

	MYVALID(device->CreateTexture(512, 512, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &text, NULL));
	MYVALID(DXCreateEffect("../media/shaders/celshading.fx", device, &effect));

	DXRenderText(HELP_TEXT, text, 512, 512);

	D3DXVECTOR3 eye(0.5f, 0.5f, -1.5f);
	D3DXVECTOR3 look(0, 0, 0);
	D3DXVECTOR3 up(0, 1, 0);

	D3DXMatrixPerspectiveFovLH(&proj, D3DX_PI / 3, (float)screenwidth / (float)screenheight, 0.1f, 10);
	D3DXMatrixLookAtLH(&view, &eye, &look, &up);
	D3DXMatrixIdentity(&world);

	return S_OK;
}
//*************************************************************************************************************
void UninitScene()
{
	SAFE_RELEASE(colorsurface);
	SAFE_RELEASE(normalsurface);
	SAFE_RELEASE(edgesurface);

	SAFE_RELEASE(colortarget);
	SAFE_RELEASE(normaltarget);
	SAFE_RELEASE(edgetarget);

	SAFE_RELEASE(vertexdecl);
	SAFE_RELEASE(mesh);
	SAFE_RELEASE(effect);
	SAFE_RELEASE(texture);
	SAFE_RELEASE(text);
	SAFE_RELEASE(intensity);
}
//*************************************************************************************************************
void KeyPress(WPARAM wparam)
{
	switch( wparam )
	{
	case 0x31:
		useedgedetect = true;
		break;

	case 0x32:
		useedgedetect = false;
		break;

	default:
		break;
	}
}
//*************************************************************************************************************
void Update(float delta)
{
}
//*************************************************************************************************************
void Render(float alpha, float elapsedtime)
{
	static float time = 0;

	D3DXMATRIX	inv;
	D3DXVECTOR3	axis(0, 1, 0);
	D3DXVECTOR4	texelsize(1.0f / (float)screenwidth, 1.0f / (float)screenheight, 0, 1);
	
	LPDIRECT3DSURFACE9 oldtarget = NULL;

	time += elapsedtime;

	D3DXMatrixRotationAxis(&inv, &axis, time);
	D3DXMatrixScaling(&world, 0.3f, 0.3f, 0.3f);
	//D3DXMatrixScaling(&world, 0.6f, 0.6f, 0.6f);
	D3DXMatrixMultiply(&world, &world, &inv);
	D3DXMatrixInverse(&inv, NULL, &world);

	device->SetTexture(0, texture);
	device->SetTexture(1, intensity);

	effect->SetMatrix("matWorld", &world);
	effect->SetMatrix("matWorldInv", &inv);
	effect->SetMatrix("matView", &view);
	effect->SetMatrix("matProj", &proj);

	if( useedgedetect )
	{
		device->GetRenderTarget(0, &oldtarget);
		device->SetRenderTarget(0, colorsurface);
		device->SetRenderTarget(1, normalsurface);
	}

	device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|D3DCLEAR_STENCIL, 0xff6694ed, 1.0f, 0);

	if( SUCCEEDED(device->BeginScene()) )
	{
		// draw scene + normals/depth
		effect->SetTechnique("celshading");

		effect->Begin(NULL, 0);
		effect->BeginPass(0);
		{
			mesh->DrawSubset(0);
		}
		effect->EndPass();
		effect->End();

		if( useedgedetect )
		{
			// edge detection
			device->SetVertexDeclaration(vertexdecl);
			device->SetRenderTarget(0, edgesurface);
			device->SetRenderTarget(1, NULL);
			device->SetTexture(0, normaltarget);
			device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff6694ed, 1.0f, 0);

			effect->SetTechnique("edgedetect");
			effect->SetVector("texelSize", &texelsize);

			effect->Begin(NULL, 0);
			effect->BeginPass(0);
			{
				device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quadvertices, sizeof(D3DXVECTOR4) + sizeof(D3DXVECTOR2));
			}
			effect->EndPass();
			effect->End();

			// put together
			device->SetRenderTarget(0, oldtarget);
			device->SetTexture(0, colortarget);
			device->SetTexture(2, edgetarget);
			device->Clear(0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0xff6694ed, 1.0f, 0);

			oldtarget->Release();
			effect->SetTechnique("final");

			effect->Begin(NULL, 0);
			effect->BeginPass(0);
			{
				device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quadvertices, sizeof(D3DXVECTOR4) + sizeof(D3DXVECTOR2));
			}
			effect->EndPass();
			effect->End();
		}
		else
		{
			D3DXMATRIX offproj;

			// use the stencil buffer
			device->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
			device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			device->SetRenderState(D3DRS_STENCILENABLE, TRUE);
			device->SetRenderState(D3DRS_ZENABLE, FALSE);

			device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			device->SetRenderState(D3DRS_STENCILREF, 1);
			device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

			device->SetTransform(D3DTS_WORLD, &world);
			device->SetTransform(D3DTS_VIEW, &view);

			float thickness = 3.5f;

			// render object 4 times with offseted frustum
			for( float i = -thickness; i < thickness + 1; i += 2 * thickness )
			{
				for( float j = -thickness; j < thickness + 1; j += 2 * thickness )
				{
					D3DXMatrixTranslation(&offproj, i / (float)screenwidth, j / (float)screenheight, 0);
					D3DXMatrixMultiply(&offproj, &proj, &offproj);

					device->SetTransform(D3DTS_PROJECTION, &offproj);
					mesh->DrawSubset(0);
				}
			}

			// erase area in the center
			device->SetRenderState(D3DRS_STENCILREF, 0);
			device->SetTransform(D3DTS_PROJECTION, &proj);

			mesh->DrawSubset(0);

			// now render outlines
			device->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA);
			device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_NOTEQUAL);
			device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
			device->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);

			device->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);

			device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CONSTANT);
			device->SetTextureStageState(0, D3DTSS_CONSTANT, 0);
			{
				device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, quadvertices, sizeof(D3DXVECTOR4) + sizeof(D3DXVECTOR2));
			}
			device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			device->SetRenderState(D3DRS_ZENABLE, TRUE);
			device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
		}

		device->SetTexture(2, NULL);

		// render text
		device->SetFVF(D3DFVF_XYZRHW|D3DFVF_TEX1);
		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		device->SetTexture(0, text);
		device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, textvertices, 6 * sizeof(float));

		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->SetRenderState(D3DRS_ZENABLE, TRUE);

		device->SetTexture(0, NULL);
		device->EndScene();
	}

	device->Present(NULL, NULL, NULL, NULL);
}
//*************************************************************************************************************
