#include "sysconfig.h"
#include "sysdeps.h"
#include "config.h"
#include "uae.h"
#include "options.h"
#include "custom.h"
#include "memory.h"
#include "blitter.h"
#include "blitfunc.h"
#include "blit.h"

#include "debug_uae4all.h"

#if !defined(SECURE_BLITTER) || defined(SAFE_MEMORY_ACCESS)
#define _CHIPMEM_WGET(PT) CHIPMEM_WGET(PT)
#define _CHIPMEM_WPUT(PT,DA) CHIPMEM_WPUT(PT,DA)
#else
#define _CHIPMEM_WGET(PT) CHIPMEM_WGET((PT)&0x000FFFFF)
#define _CHIPMEM_WPUT(PT,DA) CHIPMEM_WPUT((PT)&0x000FFFFF,DA)
#endif


#define b__vblitsize b->vblitsize
#define b__hblitsize_ b->hblitsize
#ifdef USE_BLIT_MASKTABLE
#define b__hblitsize_l b->hblitsize
#else
#define b__hblitsize_l (b->hblitsize-1)
#endif

#ifdef USE_VAR_BLITSIZE
#define _vblitsize_ b__vblitsize
#define _hblitsize_ b__hblitsize_
#define _hblitsize_l b__hblitsize_l
#endif

#ifndef USE_LARGE_BLITFUNC
void blitdofast_0 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_0 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_=b__hblitsize_;
#endif
for (j = 0; j < _vblitsize_; j++) {
	for (i = 0; i < _hblitsize_; i++) {
		uae_u32 bltadat, srca;

		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (0) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	if (ptd) ptd += b->bltdmod;
}
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_0 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_0 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_=b__hblitsize_;
#endif
for (j = 0; j < _vblitsize_; j++) {
	for (i = 0; i < _hblitsize_; i++) {
		uae_u32 bltadat, srca;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (0) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	if (ptd) ptd -= b->bltdmod;
}
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((~srca & srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_2a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_2a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_2a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_2a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & ~(srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_30 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_30 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_30 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_30 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca & ~srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_3a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_3a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_3a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_3a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb ^ (srca | (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_3c (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_3c (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_3c (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_3c (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_4a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_4a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_4a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_4a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb | srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_6a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_6a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_6a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_6a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_8a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_8a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_8a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_8a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (~srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_8c (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_8c (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_8c (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_8c (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcb & (~srca | srcc))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_9a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_9a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_9a (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_9a (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & ~srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_a8 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_a8 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_a8 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_a8 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc & (srca | srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_aa (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_aa (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_=b__hblitsize_;
#endif
for (j = 0; j < _vblitsize_; j++) {
	for (i = 0; i < _hblitsize_; i++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srcc) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_aa (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_aa (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_=b__hblitsize_;
#endif
for (j = 0; j < _vblitsize_; j++) {
	for (i = 0; i < _hblitsize_; i++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srcc) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_b1 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_b1 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_b1 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_b1 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (~(srca ^ (srcc | (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_ca (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_ca (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_ca (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_ca (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srca & (srcb ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_cc (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_cc (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_=b__hblitsize_;
#endif
for (j = 0; j < _vblitsize_; j++) {
	for (i = 0; i < _hblitsize_; i++) {
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srcb) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	if (ptb) ptb += b->bltbmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_cc (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_cc (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_=b__hblitsize_;
#endif
for (j = 0; j < _vblitsize_; j++) {
	for (i = 0; i < _hblitsize_; i++) {
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srcb) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	if (ptb) ptb -= b->bltbmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_d8 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_d8 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_d8 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_d8 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca ^ (srcc & (srca ^ srcb)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_e2 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_e2 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_e2 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_e2 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc ^ (srcb & (srca ^ srcc)))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_ea (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_ea (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_ea (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_ea (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srcc | (srca & srcb))) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_f0 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_f0 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptd) ptd += b->bltdmod;
}
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_f0 (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_f0 (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = (srca) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptd) ptd -= b->bltdmod;
}
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_fa (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_fa (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc += 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptc) ptc += b->bltcmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_fa (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_fa (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 srcc = b->bltcdat;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptc) { srcc = _CHIPMEM_WGET (ptc); ptc -= 2; }
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcc)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptc) ptc -= b->bltcmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltcdat = srcc;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_fc (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_fc (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
int i,j;
uae_u32 totald = 0;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptd) ptd += b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;

		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb += 2;
			srcb = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta += 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltalwm;
		srca = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd += 2; }
	}
#endif
	if (pta) pta += b->bltamod;
	if (ptb) ptb += b->bltbmod;
	if (ptd) ptd += b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_fc (struct bltinfo *_GCCRES_ b)
{
uaecptr pta=b->pta;
uaecptr ptb=b->ptb;
uaecptr ptc=b->ptc;
uaecptr ptd=b->ptd;
#else
void blitdofast_desc_fc (uaecptr pta, uaecptr ptb, uaecptr ptc, uaecptr ptd, struct bltinfo *_GCCRES_ b)
{
#endif
uae_u32 totald = 0;
int i,j;
uae_u32 preva = 0;
uae_u32 prevb = 0, srcb = b->bltbhold;
uae_u32 dstd=0;
uaecptr dstp = 0;
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= mask;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptd) ptd -= b->bltdmod;
} } else 
#endif
for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
	for (i = 1; i < _hblitsize_l; i++) {
#else
	for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#ifndef USE_BLIT_MASKTABLE
	{
		uae_u32 bltadat, srca;
		if (ptb) {
			uae_u32 bltbdat = _CHIPMEM_WGET (ptb); ptb -= 2;
			srcb = ((bltbdat << 16) | prevb) >> b->blitdownbshift;
			prevb = bltbdat;
		}
		if (pta) { bltadat = _CHIPMEM_WGET (pta); pta -= 2; } else { bltadat = b->bltadat; }
		bltadat &= b->bltafwm;
		srca = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
		dstd = ((srca | srcb)) & 0xFFFF;
		totald |= dstd;
		if (ptd) { dstp = ptd; ptd -= 2; }
	}
#endif
	if (pta) pta -= b->bltamod;
	if (ptb) ptb -= b->bltbmod;
	if (ptd) ptd -= b->bltdmod;
}
b->bltbhold = srcb;
		if (dstp) _CHIPMEM_WPUT (dstp, dstd);
if (totald != 0) b->blitzero = 0;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_fill (struct bltinfo *_GCCRES_ b)
{
uaecptr bltadatptr=b->pta;
uaecptr bltbdatptr=b->ptb;
uaecptr bltcdatptr=b->ptc;
uaecptr bltddatptr=b->ptd;
#else
void blitdofast_fill (uaecptr bltadatptr, uaecptr bltbdatptr, uaecptr bltcdatptr, uaecptr bltddatptr, struct bltinfo *_GCCRES_ b)
{
#endif
	int i,j;
	uae_u32 blitbhold = b->bltbhold;
	uae_u32 preva = 0, prevb = 0;
	uaecptr dstp = 0;
	const uae_u8 mt = bltcon0 & 0xFF;
	extern uae_u8 blit_filltable[256][4][2];

#ifdef DEBUG_BLITTER
	dbgf("bltbhold=0x%X, vblitsize=0x%X, bltcon1=0x%X\n",b->bltbhold,b->vblitsize,bltcon1);
#endif
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr += 2;
		} else
		    bltadat = b->bltadat;
		bltadat &= mask;
		blitahold = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;

		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr += 2;
		    blitbhold = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr += 2;
		}
		if (dstp)
		{
			_CHIPMEM_WPUT (dstp, b->bltddat);
		}
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr += 2;
		}
	    if (bltadatptr) bltadatptr += b->bltamod;
	    if (bltbdatptr) bltbdatptr += b->bltbmod;
	    if (bltcdatptr) bltcdatptr += b->bltcmod;
	    if (bltddatptr) bltddatptr += b->bltdmod;
} } else 
#endif
	/*if (!b->blitfill) write_log ("minterm %x not present\n",mt); */
	for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	    {
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr += 2;
		} else
		    bltadat = b->bltadat;
		bltadat &= b->bltafwm;
		blitahold = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;

		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr += 2;
		    blitbhold = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr += 2;
		}
		if (dstp)
		{
			_CHIPMEM_WPUT (dstp, b->bltddat);
		}
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr += 2;
		}
	    }
	    for (i = 1; i < _hblitsize_l; i++) {
#else
	    for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr += 2;
		} else
		    bltadat = b->bltadat;
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		blitahold = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;

		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr += 2;
		    blitbhold = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr += 2;
		}
		if (dstp)
		{
			_CHIPMEM_WPUT (dstp, b->bltddat);
		}
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr += 2;
		}
	    }
#ifndef USE_BLIT_MASKTABLE
	    {
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr += 2;
		} else
		    bltadat = b->bltadat;
		bltadat &= b->bltalwm;
		blitahold = (((uae_u32)preva << 16) | bltadat) >> b->blitashift;
		preva = bltadat;

		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr += 2;
		    blitbhold = (((uae_u32)prevb << 16) | bltbdat) >> b->blitbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr += 2;
		}
		if (dstp)
		{
			_CHIPMEM_WPUT (dstp, b->bltddat);
		}
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr += 2;
		}
	    }
#endif
	    if (bltadatptr) bltadatptr += b->bltamod;
	    if (bltbdatptr) bltbdatptr += b->bltbmod;
	    if (bltcdatptr) bltcdatptr += b->bltcmod;
	    if (bltddatptr) bltddatptr += b->bltdmod;
	}
	if (dstp)
	{
		_CHIPMEM_WPUT (dstp, b->bltddat);
	}
	b->bltbhold = blitbhold;
}

#ifndef USE_LARGE_BLITFUNC
void blitdofast_desc_fill (struct bltinfo *_GCCRES_ b)
{
uaecptr bltadatptr=b->pta;
uaecptr bltbdatptr=b->ptb;
uaecptr bltcdatptr=b->ptc;
uaecptr bltddatptr=b->ptd;
#else
void blitdofast_desc_fill (uaecptr bltadatptr, uaecptr bltbdatptr, uaecptr bltcdatptr, uaecptr bltddatptr, struct bltinfo *_GCCRES_ b)
{
#endif
	int i,j;
	uae_u32 blitbhold = b->bltbhold;
	uae_u32 preva = 0, prevb = 0;
	uaecptr dstp = 0;
	const uae_u8 mt = bltcon0 & 0xFF;
	extern uae_u8 blit_filltable[256][4][2];

#ifdef DEBUG_BLITTER
	dbgf("bltbhold=0x%X, vblitsize=0x%X, bltcon1=0x%X\n",b->bltbhold,b->vblitsize,bltcon1);
#endif
/*	if (!b->blitfill) write_log ("minterm %x not present\n",mt);*/
#ifndef USE_VAR_BLITSIZE
const int _vblitsize_=b__vblitsize;
const int _hblitsize_l=b__hblitsize_l;
#endif
#ifndef USE_BLIT_MASKTABLE
if (!_hblitsize_l){ 
const  uae_u32 mask=b->bltafwm&b->bltalwm;
for (j = 0; j < _vblitsize_; j++) {
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr -= 2;
		} else
		    bltadat = b->bltadat;
		bltadat &= mask;
		blitahold = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr -= 2;
		    blitbhold = (((uae_u32)bltbdat << 16) | prevb) >> b->blitdownbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr -= 2;
		}
		if (dstp) _CHIPMEM_WPUT (dstp, b->bltddat);
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr -= 2;
		}
	    if (bltadatptr) bltadatptr -= b->bltamod;
	    if (bltbdatptr) bltbdatptr -= b->bltbmod;
	    if (bltcdatptr) bltcdatptr -= b->bltcmod;
	    if (bltddatptr) bltddatptr -= b->bltdmod;
} } else 
#endif
	for (j = 0; j < _vblitsize_; j++) {
#ifndef USE_BLIT_MASKTABLE
	    {
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr -= 2;
		} else
		    bltadat = b->bltadat;
		bltadat &= b->bltafwm;
		blitahold = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr -= 2;
		    blitbhold = (((uae_u32)bltbdat << 16) | prevb) >> b->blitdownbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr -= 2;
		}
		if (dstp) _CHIPMEM_WPUT (dstp, b->bltddat);
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr -= 2;
		}
	    }
	    for (i = 1; i < _hblitsize_l; i++) {
#else
	    for (i = 0; i < _hblitsize_l; i++) {
#endif
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr -= 2;
		} else
		    bltadat = b->bltadat;
#ifndef USE_BLIT_MASKTABLE
//		bltadat &= 0xFFFF;
#else
		bltadat &= blit_masktable[i];
#endif
		blitahold = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr -= 2;
		    blitbhold = (((uae_u32)bltbdat << 16) | prevb) >> b->blitdownbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr -= 2;
		}
		if (dstp) _CHIPMEM_WPUT (dstp, b->bltddat);
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr -= 2;
		}
	    }
#ifndef USE_BLIT_MASKTABLE
	    {
		uae_u32 bltadat, blitahold;
		if (bltadatptr) {
		    bltadat = _CHIPMEM_WGET (bltadatptr);
		    bltadatptr -= 2;
		} else
		    bltadat = b->bltadat;
		bltadat &= b->bltalwm;
		blitahold = (((uae_u32)bltadat << 16) | preva) >> b->blitdownashift;
		preva = bltadat;
		if (bltbdatptr) {
		    uae_u16 bltbdat = _CHIPMEM_WGET (bltbdatptr);
		    bltbdatptr -= 2;
		    blitbhold = (((uae_u32)bltbdat << 16) | prevb) >> b->blitdownbshift;
		    prevb = bltbdat;
		}
		if (bltcdatptr) {
		    b->bltcdat = _CHIPMEM_WGET (bltcdatptr);
		    bltcdatptr -= 2;
		}
		if (dstp) _CHIPMEM_WPUT (dstp, b->bltddat);
		b->bltddat = blit_func (blitahold, blitbhold, b->bltcdat, mt) & 0xFFFF;
		if (b->blitfill) {
		    uae_u16 d = b->bltddat;
		    int ifemode = b->blitife ? 2 : 0;
		    int fc1 = blit_filltable[d & 255][ifemode + b->blitfc][1];
		    b->bltddat = (blit_filltable[d & 255][ifemode + b->blitfc][0]
					+ (blit_filltable[d >> 8][ifemode + fc1][0] << 8));
		    b->blitfc = blit_filltable[d >> 8][ifemode + fc1][1];
		}
		if (b->bltddat)
		    b->blitzero = 0;
		if (bltddatptr) {
		    dstp = bltddatptr;
		    bltddatptr -= 2;
		}
	    }
#endif
	    if (bltadatptr) bltadatptr -= b->bltamod;
	    if (bltbdatptr) bltbdatptr -= b->bltbmod;
	    if (bltcdatptr) bltcdatptr -= b->bltcmod;
	    if (bltddatptr) bltddatptr -= b->bltdmod;
	}
	if (dstp) _CHIPMEM_WPUT (dstp, b->bltddat);
	b->bltbhold = blitbhold;
}

