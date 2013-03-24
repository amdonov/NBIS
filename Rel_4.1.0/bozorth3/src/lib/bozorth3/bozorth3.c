/*******************************************************************************

License: 
This software and/or related materials was developed at the National Institute
of Standards and Technology (NIST) by employees of the Federal Government
in the course of their official duties. Pursuant to title 17 Section 105
of the United States Code, this software is not subject to copyright
protection and is in the public domain. 

This software and/or related materials have been determined to be not subject
to the EAR (see Part 734.3 of the EAR for exact details) because it is
a publicly available technology and software, and is freely distributed
to any interested party with no licensing requirements.  Therefore, it is 
permissible to distribute this software as a free download from the internet.

Disclaimer: 
This software and/or related materials was developed to promote biometric
standards and biometric technology testing for the Federal Government
in accordance with the USA PATRIOT Act and the Enhanced Border Security
and Visa Entry Reform Act. Specific hardware and software products identified
in this software were used in order to perform the software development.
In no case does such identification imply recommendation or endorsement
by the National Institute of Standards and Technology, nor does it imply that
the products and equipment identified are necessarily the best available
for the purpose.

This software and/or related materials are provided "AS-IS" without warranty
of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
licensed product, however used. In no event shall NIST be liable for any
damages and/or costs, including but not limited to incidental or consequential
damages of any kind, including economic damage or injury to property and lost
profits, regardless of whether NIST shall be advised, have reason to know,
or in fact shall know of the possibility.

By using this software, you agree to bear all risk relating to quality,
use and performance of the software and/or related materials.  You agree
to hold the Government harmless from any claim arising from your use
of the software.

*******************************************************************************/

/***********************************************************************
      LIBRARY: FING - NIST Fingerprint Systems Utilities

      FILE:           BOZORTH3.C
      ALGORITHM:      Allan S. Bozorth (FBI)
      MODIFICATIONS:  Michael D. Garris (NIST)
                      Stan Janet (NIST)
      DATE:           09/21/2004

      Contains the "core" routines responsible for supporting the
      Bozorth3 fingerprint matching algorithm.

***********************************************************************

      ROUTINES:
#cat: bz_comp -  takes a set of minutiae (probe or gallery) and
#cat:            compares/measures  each minutia's {x,y,t} with every
#cat:            other minutia's {x,y,t} in the set creating a table
#cat:            of pairwise comparison entries
#cat: bz_find -  trims sorted table of pairwise minutia comparisons to
#cat:            a max distance of 75^2
#cat: bz_match - takes the two pairwise minutia comparison tables (a probe
#cat:            table and a gallery table) and compiles a list of
#cat:            all relatively "compatible" entries between the two
#cat:            tables generating a match table
#cat: bz_match_score - takes a match table and traverses it looking for
#cat:            a sufficiently long path (or a cluster of compatible paths)
#cat:            of "linked" match table entries
#cat:            the accumulation of which results in a match "score"
#cat: bz_sift -  main routine handling the path linking and match table
#cat:            traversal
#cat: bz_final_loop - (declared static) a final postprocess after
#cat:            the main match table traversal which looks to combine
#cat:            clusters of compatible paths

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <bozorth.h>

/***********************************************************************/
void
bz_comp (int npoints,		/* INPUT: # of points */
	 int xcol[MAX_BOZORTH_MINUTIAE],	/* INPUT: x cordinates */
	 int ycol[MAX_BOZORTH_MINUTIAE],	/* INPUT: y cordinates */
	 int thetacol[MAX_BOZORTH_MINUTIAE],	/* INPUT: theta values */
	 int *ncomparisons,	/* OUTPUT: number of pointwise comparisons */
	 int cols[][COLS_SIZE_2],	/* OUTPUT: pointwise comparison table */
	 int *colptrs[]		/* INPUT and OUTPUT: sorted list of pointers to rows in cols[] */
  )
{
  int i, j, k;

  int b;
  int t;
  int n;
  int l;

  int table_index;

  int dx;
  int dy;
  int distance;

  int theta_kj;
  int beta_j;
  int beta_k;

  int *c;



  c = &cols[0][0];

  table_index = 0;
  for (k = 0; k < npoints - 1; k++)
    {
      for (j = k + 1; j < npoints; j++)
	{


	  if (thetacol[j] > 0)
	    {

	      if (thetacol[k] == thetacol[j] - 180)
		continue;
	    }
	  else
	    {

	      if (thetacol[k] == thetacol[j] + 180)
		continue;
	    }


	  dx = xcol[j] - xcol[k];
	  dy = ycol[j] - ycol[k];
	  distance = SQUARED (dx) + SQUARED (dy);
	  if (distance > SQUARED (DM))
	    {
	      if (dx > DM)
		break;
	      else
		continue;

	    }

	  /* The distance is in the range [ 0, 125^2 ] */
	  if (dx == 0)
	    theta_kj = 90;
	  else
	    {
	      double dz;

	      if (m1_xyt)
		dz = (180.0F / PI_SINGLE) * atanf ((float) -dy / (float) dx);
	      else
		dz = (180.0F / PI_SINGLE) * atanf ((float) dy / (float) dx);
	      if (dz < 0.0F)
		dz -= 0.5F;
	      else
		dz += 0.5F;
	      theta_kj = (int) dz;
	    }


	  beta_k = theta_kj - thetacol[k];
	  beta_k = IANGLE180 (beta_k);

	  beta_j = theta_kj - thetacol[j] + 180;
	  beta_j = IANGLE180 (beta_j);


	  if (beta_k < beta_j)
	    {
	      *c++ = distance;
	      *c++ = beta_k;
	      *c++ = beta_j;
	      *c++ = k + 1;
	      *c++ = j + 1;
	      *c++ = theta_kj;
	    }
	  else
	    {
	      *c++ = distance;
	      *c++ = beta_j;
	      *c++ = beta_k;
	      *c++ = k + 1;
	      *c++ = j + 1;
	      *c++ = theta_kj + 400;

	    }






	  b = 0;
	  t = table_index + 1;
	  l = 1;
	  n = -1;		/* Init binary search state ... */




	  while (t - b > 1)
	    {
	      int *midpoint;

	      l = (b + t) / 2;
	      midpoint = colptrs[l - 1];




	      for (i = 0; i < 3; i++)
		{
		  int dd, ff;

		  dd = cols[table_index][i];

		  ff = midpoint[i];


		  n = SENSE (dd, ff);


		  if (n < 0)
		    {
		      t = l;
		      break;
		    }
		  if (n > 0)
		    {
		      b = l;
		      break;
		    }
		}

	      if (n == 0)
		{
		  n = 1;
		  b = l;
		}
	    }			/* END while */

	  if (n == 1)
	    ++l;




	  for (i = table_index; i >= l; --i)
	    colptrs[i] = colptrs[i - 1];


	  colptrs[l - 1] = &cols[table_index][0];
	  ++table_index;


	  if (table_index == 19999)
	    {
#ifndef NOVERBOSE
	      if (verbose_bozorth)
		printf ("bz_comp(): breaking loop to avoid table overflow\n");
#endif
	      goto COMP_END;
	    }

	}			/* END for j */

    }				/* END for k */

COMP_END:
  *ncomparisons = table_index;

}

/***********************************************************************/
void
bz_find (int *xlim,		/* INPUT:  number of pointwise comparisons in table */
	 /* OUTPUT: determined insertion location (NOT ALWAYS SET) */
	 int *colpt[]		/* INOUT:  sorted list of pointers to rows in the pointwise comparison table */
  )
{
  int midpoint;
  int top;
  int bottom;
  int state;
  int distance;



/* binary search to locate the insertion location of a predefined distance in list of sorted distances */


  bottom = 0;
  top = *xlim + 1;
  midpoint = 1;
  state = -1;

  while (top - bottom > 1)
    {
      midpoint = (bottom + top) / 2;
      distance = *colpt[midpoint - 1];
      state = SENSE_NEG_POS (FD, distance);
      if (state < 0)
	top = midpoint;
      else
	{
	  bottom = midpoint;
	}
    }

  if (state > -1)
    ++midpoint;

  if (midpoint < *xlim)
    *xlim = midpoint;



}

/***********************************************************************/
/* Make room in RTP list at insertion point by shifting contents down the
   list.  Then insert the address of the current ROT row into desired
   location */
/***********************************************************************/
static void
rtp_insert (int *rtp[], int l, int idx, int *ptr)
{
  int shiftcount;
  int **r1;
  int **r2;


  r1 = &rtp[idx];
  r2 = r1 - 1;

  shiftcount = (idx - l) + 1;
  while (shiftcount-- > 0)
    {
      *r1-- = *r2--;
    }
  *r1 = ptr;
}

/***********************************************************************/
/* Builds list of compatible edge pairs between the 2 Webs. */
/* The Edge pair DeltaThetaKJs and endpoints are sorted     */
/*	first on Subject's K,                               */
/*	then On-File's J or K (depending),                  */
/*	and lastly on Subject's J point index.              */
/* Return value is the # of compatible edge pairs           */
/***********************************************************************/
int
bz_match (struct bz_data_struct *pbz_data, int probe_ptrlist_len,	/* INPUT:  pruned length of Subject's pointer list */
	  int gallery_ptrlist_len	/* INPUT:  pruned length of On-File Record's pointer list */
  )
{
  int i;			/* Temp index */
  int ii;			/* Temp index */
  int edge_pair_index;		/* Compatible edge pair index */
  float dz;			/* Delta difference and delta angle stats */
  float fi;			/* Distance limit based on factor TK */
  int *ss;			/* Subject's comparison stats row */
  int *ff;			/* On-File Record's comparison stats row */
  int j;			/* On-File Record's row index */
  int k;			/* Subject's row index */
  int st;			/* Starting On-File Record's row index */
  int p1;			/* Adjusted Subject's ThetaKJ, DeltaThetaKJs, K or J point index */
  int p2;			/* Adjusted On-File's ThetaKJ, RTP point index */
  int n;			/* ThetaKJ and binary search state variable */
  int l;			/* Midpoint of binary search */
  int b;			/* ThetaKJ state variable, and bottom of search range */
  int t;			/* Top of search range */

  int *rotptr;


#define ROT_SIZE_1 20000
#define ROT_SIZE_2 5

  int (*rot)[ROT_SIZE_2];
  int ind;
  rot = malloc (ROT_SIZE_2 * ROT_SIZE_1 * sizeof (int));
  if (rot == NULL)
    {
      fprintf (stderr, "out of memory\n");
      return -1;
    }

  int *rtp[ROT_SIZE_1];

/* These now externally defined in bozorth.h */
/* extern int * scolpt[ SCOLPT_SIZE ];			 INPUT */
/* extern int * fcolpt[ FCOLPT_SIZE ];			 INPUT */
/* extern int   colp[ COLP_SIZE_1 ][ COLP_SIZE_2 ];	 OUTPUT */
/* extern int verbose_bozorth; */
/* extern FILE * errorfp; */
/* extern char * get_progname( void ); */
/* extern char * get_probe_filename( void ); */
/* extern char * get_gallery_filename( void ); */

  st = 1;
  edge_pair_index = 0;
  rotptr = &rot[0][0];

/* Foreach sorted edge in Subject's Web ... */

  for (k = 1; k < probe_ptrlist_len; k++)
    {
      ss = pbz_data->scolpt[k - 1];

      /* Foreach sorted edge in On-File Record's Web ... */

      for (j = st; j <= gallery_ptrlist_len; j++)
	{
	  ff = pbz_data->fcolpt[j - 1];
	  dz = *ff - *ss;

	  fi = (2.0F * TK) * (*ff + *ss);


	  if (SQUARED (dz) > SQUARED (fi))
	    {
	      if (dz < 0)
		{

		  st = j + 1;

		  continue;
		}
	      else
		break;


	    }



	  for (i = 1; i < 3; i++)
	    {
	      float dz_squared;

	      dz = *(ss + i) - *(ff + i);
	      dz_squared = SQUARED (dz);

	      if (dz_squared > TXS && dz_squared < CTXS)
		break;
	    }

	  if (i < 3)
	    continue;

	  if (*(ss + 5) >= 220)
	    {
	      p1 = *(ss + 5) - 580;
	      n = 1;
	    }
	  else
	    {
	      p1 = *(ss + 5);
	      n = 0;
	    }


	  if (*(ff + 5) >= 220)
	    {
	      p2 = *(ff + 5) - 580;
	      b = 1;
	    }
	  else
	    {
	      p2 = *(ff + 5);
	      b = 0;
	    }

	  p1 -= p2;
	  p1 = IANGLE180 (p1);

	  if (n != b)
	    {

	      *rotptr++ = p1;
	      *rotptr++ = *(ss + 3);
	      *rotptr++ = *(ss + 4);

	      *rotptr++ = *(ff + 4);
	      *rotptr++ = *(ff + 3);
	    }
	  else
	    {
	      *rotptr++ = p1;
	      *rotptr++ = *(ss + 3);
	      *rotptr++ = *(ss + 4);

	      *rotptr++ = *(ff + 3);
	      *rotptr++ = *(ff + 4);
	    }

	  n = -1;
	  l = 1;
	  b = 0;
	  t = edge_pair_index + 1;
	  while (t - b > 1)
	    {
	      l = (b + t) / 2;

	      for (i = 0; i < 3; i++)
		{
		  int ii_table[] = { 1, 3, 2 };

		  /*      1 = Subject's Kth, */
		  /*      3 = On-File's Jth or Kth (depending), */
		  /*      2 = Subject's Jth */

		  ii = ii_table[i];
		  p1 = rot[edge_pair_index][ii];
		  p2 = *(rtp[l - 1] + ii);

		  n = SENSE (p1, p2);

		  if (n < 0)
		    {
		      t = l;
		      break;
		    }
		  if (n > 0)
		    {
		      b = l;
		      break;
		    }
		}

	      if (n == 0)
		{
		  n = 1;
		  b = l;
		}
	    }			/* END while() for binary search */


	  if (n == 1)
	    ++l;

	  rtp_insert (rtp, l, edge_pair_index, &rot[edge_pair_index][0]);
	  ++edge_pair_index;

	  if (edge_pair_index == 19999)
	    {
#ifndef NOVERBOSE
	      if (verbose_bozorth)
		fprintf (errorfp,
			 "%s: bz_match(): WARNING: list is full, breaking loop early [p=%s; g=%s]\n",
			 get_progname (), get_probe_filename (),
			 get_gallery_filename ());
#endif
	      goto END;		/* break out if list exceeded */
	    }

	}			/* END FOR On-File (edge) distance */

    }				/* END FOR Subject (edge) distance */



END:
  {
    int *colp_ptr = &pbz_data->colp[0][0];

    for (i = 0; i < edge_pair_index; i++)
      {
	INT_COPY (colp_ptr, rtp[i], COLP_SIZE_2);


      }
  }


  free (rot);

  return edge_pair_index;	/* Return the number of compatible edge pairs stored into colp[][] */
}

/**************************************************************************/
/* These global arrays are declared "static" as they are only used        */
/* between bz_match_score() & bz_final_loop()                             */
/**************************************************************************/

static int bz_final_loop (struct bz_data_struct *, int, int[CT_SIZE],
			  int[GCT_SIZE], int[CTT_SIZE], int **, int ***);

/**************************************************************************/
int
bz_match_score (struct bz_data_struct *pbz_data, int np,
		struct xyt_struct *pstruct, struct xyt_struct *gstruct)
{
  int ct[CT_SIZE];
  int gct[GCT_SIZE];
  int *ctt;
  ctt = malloc (sizeof (int) * CTT_SIZE);
  int **ctp;
  ctp = malloc (sizeof (int *) * CTP_SIZE_1);
  int ind;
  for (ind = 0; ind < CTP_SIZE_1; ind++)
    {
      ctp[ind] = malloc (sizeof (int) * CTP_SIZE_2);
    }
  int ***yy;
  yy = malloc (sizeof (int **) * YY_SIZE_1);
  for (ind = 0; ind < YY_SIZE_1; ind++)
    {
      yy[ind] = malloc (sizeof (int *) * YY_SIZE_2);
      int ind2;
      for (ind2 = 0; ind2 < YY_SIZE_2; ind2++)
	{
	  yy[ind][ind2] = malloc (sizeof (int) * YY_SIZE_3);
	  int ind3;
	  for (ind3 = 0; ind3 < YY_SIZE_3; ind3++)
	    {
	      yy[ind][ind2][ind3] = 0;
	    }
	}
    }
  int kx, kq;
  int ftt;
  int tot;
  int qh;
  int tp;
  int ll, jj, kk, n, t, b;
  int k, i, j, ii, z;
  int kz, l;
  int p1, p2;
  int dw, ww;
  int match_score;
  int qq_overflow = 0;
  float fi;

/* These next 3 arrays originally declared global, but moved here */
/* locally because they are only used herein */
  int rr[RR_SIZE];
  int avn[AVN_SIZE];
  int avv[AVV_SIZE_1][AVV_SIZE_2];

/* These now externally defined in bozorth.h */
/* extern FILE * errorfp; */
/* extern char * get_progname( void ); */
/* extern char * get_probe_filename( void ); */
/* extern char * get_gallery_filename( void ); */






  if (pstruct->nrows < min_computable_minutiae)
    {
#ifndef NOVERBOSE
      if (gstruct->nrows < min_computable_minutiae)
	{
	  if (verbose_bozorth)
	    fprintf (errorfp,
		     "%s: bz_match_score(): both probe and gallery file have too few minutiae (%d,%d) to compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
		     get_progname (), pstruct->nrows, gstruct->nrows,
		     min_computable_minutiae, get_probe_filename (),
		     get_gallery_filename ());
	}
      else
	{
	  if (verbose_bozorth)
	    fprintf (errorfp,
		     "%s: bz_match_score(): probe file has too few minutiae (%d) to compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
		     get_progname (), pstruct->nrows,
		     min_computable_minutiae, get_probe_filename (),
		     get_gallery_filename ());
	}
#endif
      return ZERO_MATCH_SCORE;
    }



  if (gstruct->nrows < min_computable_minutiae)
    {
#ifndef NOVERBOSE
      if (verbose_bozorth)
	fprintf (errorfp,
		 "%s: bz_match_score(): gallery file has too few minutiae (%d) to compute a real Bozorth match score; min. is %d [p=%s; g=%s]\n",
		 get_progname (), gstruct->nrows, min_computable_minutiae,
		 get_probe_filename (), get_gallery_filename ());
#endif
      return ZERO_MATCH_SCORE;
    }









  /* initialize tables to 0's */
  INT_SET ((int *) &(pbz_data->yl), YL_SIZE_1 * YL_SIZE_2, 0);



  INT_SET ((int *) &(pbz_data->sc), SC_SIZE, 0);
  INT_SET ((int *) &(pbz_data->cp), CP_SIZE, 0);
  INT_SET ((int *) &(pbz_data->rp), RP_SIZE, 0);
  INT_SET ((int *) &(pbz_data->tq), TQ_SIZE, 0);
  INT_SET ((int *) &(pbz_data->rq), RQ_SIZE, 0);
  INT_SET ((int *) &(pbz_data->zz), ZZ_SIZE, 1000);	/* zz[] initialized to 1000's */

  INT_SET ((int *) &avn, AVN_SIZE, 0);	/* avn[0...4] <== 0; */





  tp = 0;
  p1 = 0;
  tot = 0;
  ftt = 0;
  kx = 0;
  match_score = 0;

  for (k = 0; k < np - 1; k++)
    {
      /* printf( "compute(): looping with k=%d\n", k ); */

      if (pbz_data->sc[k])	/* If SC counter for current pair already incremented ... */
	continue;		/*              Skip to next pair */


      i = pbz_data->colp[k][1];
      t = pbz_data->colp[k][3];




      pbz_data->qq[0] = i;
      pbz_data->rq[t - 1] = i;
      pbz_data->tq[i - 1] = t;


      ww = 0;
      dw = 0;

      do
	{
	  ftt++;
	  tot = 0;
	  qh = 1;
	  kx = k;




	  do
	    {









	      kz = pbz_data->colp[kx][2];
	      l = pbz_data->colp[kx][4];
	      kx++;
	      bz_sift (pbz_data, &ww, kz, &qh, l, kx, ftt, &tot,
		       &qq_overflow);
	      if (qq_overflow)
		{
		  fprintf (errorfp,
			   "%s: WARNING: bz_match_score(): qq[] overflow from bz_sift() #1 [p=%s; g=%s]\n",
			   get_progname (), get_probe_filename (),
			   get_gallery_filename ());
		  return QQ_OVERFLOW_SCORE;
		}

#ifndef NOVERBOSE
	      if (verbose_bozorth)
		printf ("x1 %d %d %d %d %d %d\n", kx, pbz_data->colp[kx][0],
			pbz_data->colp[kx][1], pbz_data->colp[kx][2],
			pbz_data->colp[kx][3], pbz_data->colp[kx][4]);
#endif

	    }
	  while (pbz_data->colp[kx][3] == pbz_data->colp[k][3]
		 && pbz_data->colp[kx][1] == pbz_data->colp[k][1]);
	  /* While the startpoints of lookahead edge pairs are the same as the starting points of the */
	  /* current pair, set KQ to lookahead edge pair index where above bz_sift() loop left off */

	  kq = kx;

	  for (j = 1; j < qh; j++)
	    {
	      for (i = kq; i < np; i++)
		{

		  for (z = 1; z < 3; z++)
		    {
		      if (z == 1)
			{
			  if ((j + 1) > QQ_SIZE)
			    {
			      fprintf (errorfp,
				       "%s: WARNING: bz_match_score(): qq[] overflow #1 in bozorth3(); j-1 is %d [p=%s; g=%s]\n",
				       get_progname (), j - 1,
				       get_probe_filename (),
				       get_gallery_filename ());
			      return QQ_OVERFLOW_SCORE;
			    }
			  p1 = pbz_data->qq[j];
			}
		      else
			{
			  p1 = pbz_data->tq[p1 - 1];

			}






		      if (pbz_data->colp[i][2 * z] != p1)
			break;
		    }


		  if (z == 3)
		    {
		      z = pbz_data->colp[i][1];
		      l = pbz_data->colp[i][3];



		      if (z != pbz_data->colp[k][1]
			  && l != pbz_data->colp[k][3])
			{
			  kx = i + 1;
			  bz_sift (pbz_data, &ww, z, &qh, l, kx, ftt, &tot,
				   &qq_overflow);
			  if (qq_overflow)
			    {
			      fprintf (errorfp,
				       "%s: WARNING: bz_match_score(): qq[] overflow from bz_sift() #2 [p=%s; g=%s]\n",
				       get_progname (),
				       get_probe_filename (),
				       get_gallery_filename ());
			      return QQ_OVERFLOW_SCORE;
			    }
			}
		    }
		}		/* END for i */



	      /* Done looking ahead for current j */





	      l = 1;
	      t = np + 1;
	      b = kq;

	      while (t - b > 1)
		{
		  l = (b + t) / 2;

		  for (i = 1; i < 3; i++)
		    {

		      if (i == 1)
			{
			  if ((j + 1) > QQ_SIZE)
			    {
			      fprintf (errorfp,
				       "%s: WARNING: bz_match_score(): qq[] overflow #2 in bozorth3(); j-1 is %d [p=%s; g=%s]\n",
				       get_progname (), j - 1,
				       get_probe_filename (),
				       get_gallery_filename ());
			      return QQ_OVERFLOW_SCORE;
			    }
			  p1 = pbz_data->qq[j];
			}
		      else
			{
			  p1 = pbz_data->tq[p1 - 1];
			}



		      p2 = pbz_data->colp[l - 1][i * 2 - 1];

		      n = SENSE (p1, p2);

		      if (n < 0)
			{
			  t = l;
			  break;
			}
		      if (n > 0)
			{
			  b = l;
			  break;
			}
		    }

		  if (n == 0)
		    {






		      /* Locates the head of consecutive sequence of edge pairs all having the same starting Subject and On-File edgepoints */
		      while (pbz_data->colp[l - 2][3] == p2
			     && pbz_data->colp[l - 2][1] ==
			     pbz_data->colp[l - 1][1])
			l--;

		      kx = l - 1;


		      do
			{
			  kz = pbz_data->colp[kx][2];
			  l = pbz_data->colp[kx][4];
			  kx++;
			  bz_sift (pbz_data, &ww, kz, &qh, l, kx, ftt, &tot,
				   &qq_overflow);
			  if (qq_overflow)
			    {
			      fprintf (errorfp,
				       "%s: WARNING: bz_match_score(): qq[] overflow from bz_sift() #3 [p=%s; g=%s]\n",
				       get_progname (),
				       get_probe_filename (),
				       get_gallery_filename ());
			      return QQ_OVERFLOW_SCORE;
			    }
			}
		      while (pbz_data->colp[kx][3] == p2
			     && pbz_data->colp[kx][1] ==
			     pbz_data->colp[kx - 1][1]);

		      break;
		    }		/* END if ( n == 0 ) */

		}		/* END while */

	    }			/* END for j */




	  if (tot >= MSTR)
	    {
	      jj = 0;
	      kk = 0;
	      n = 0;
	      l = 0;

	      for (i = 0; i < tot; i++)
		{


		  int colp_value = pbz_data->colp[pbz_data->y[i] - 1][0];
		  if (colp_value < 0)
		    {
		      kk += colp_value;
		      n++;
		    }
		  else
		    {
		      jj += colp_value;
		      l++;
		    }
		}


	      if (n == 0)
		{
		  n = 1;
		}
	      else if (l == 0)
		{
		  l = 1;
		}



	      fi = (float) jj / (float) l - (float) kk / (float) n;

	      if (fi > 180.0F)
		{
		  fi = (jj + kk + n * 360) / (float) tot;
		  if (fi > 180.0F)
		    fi -= 360.0F;
		}
	      else
		{
		  fi = (jj + kk) / (float) tot;
		}

	      jj = ROUND (fi);
	      if (jj <= -180)
		jj += 360;



	      kk = 0;
	      for (i = 0; i < tot; i++)
		{
		  int diff = pbz_data->colp[pbz_data->y[i] - 1][0] - jj;
		  j = SQUARED (diff);




		  if (j > TXS && j < CTXS)
		    kk++;
		  else
		    pbz_data->y[i - kk] = pbz_data->y[i];
		}		/* END FOR i */

	      tot -= kk;	/* Adjust the total edge pairs TOT based on # of edge pairs skipped */

	    }			/* END if ( tot >= MSTR ) */




	  if (tot < MSTR)
	    {




	      for (i = tot - 1; i >= 0; i--)
		{
		  int idx = pbz_data->y[i] - 1;
		  if (pbz_data->rk[idx] == 0)
		    {
		      pbz_data->sc[idx] = -1;
		    }
		  else
		    {
		      pbz_data->sc[idx] = pbz_data->rk[idx];
		    }
		}
	      ftt--;

	    }
	  else
	    {			/* tot >= MSTR */
	      /* Otherwise size of TOT group (seq. of TOT indices stored in Y) is large enough to analyze */

	      int pa = 0;
	      int pb = 0;
	      int pc = 0;
	      int pd = 0;

	      for (i = 0; i < tot; i++)
		{
		  int idx = pbz_data->y[i] - 1;
		  for (ii = 1; ii < 4; ii++)
		    {




		      kk = (SQUARED (ii) - ii + 2) / 2 - 1;




		      jj = pbz_data->colp[idx][kk];

		      switch (ii)
			{
			case 1:
			  if (pbz_data->colp[idx][0] < 0)
			    {
			      pd += pbz_data->colp[idx][0];
			      pb++;
			    }
			  else
			    {
			      pa += pbz_data->colp[idx][0];
			      pc++;
			    }
			  break;
			case 2:
			  avn[ii - 1] += pstruct->xcol[jj - 1];
			  avn[ii] += pstruct->ycol[jj - 1];
			  break;
			default:
			  avn[ii] += gstruct->xcol[jj - 1];
			  avn[ii + 1] += gstruct->ycol[jj - 1];
			  break;
			}	/* switch */
		    }		/* END for ii = [1..3] */

		  for (ii = 0; ii < 2; ii++)
		    {
		      n = -1;
		      l = 1;

		      for (jj = 1; jj < 3; jj++)
			{

			  p1 = pbz_data->colp[idx][2 * ii + jj];


			  b = 0;
			  t = pbz_data->yl[ii][tp] + 1;

			  while (t - b > 1)
			    {
			      l = (b + t) / 2;
			      p2 = yy[l - 1][ii][tp];
			      n = SENSE (p1, p2);

			      if (n < 0)
				{
				  t = l;
				}
			      else
				{
				  if (n > 0)
				    {
				      b = l;
				    }
				  else
				    {
				      break;
				    }
				}
			    }	/* END WHILE */

			  if (n != 0)
			    {
			      if (n == 1)
				++l;

			      for (kk = pbz_data->yl[ii][tp]; kk >= l; --kk)
				{
				  yy[kk][ii][tp] = yy[kk - 1][ii][tp];
				}

			      ++pbz_data->yl[ii][tp];
			      yy[l - 1][ii][tp] = p1;


			    }	/* END if ( n != 0 ) */

			  /* Otherwise, edgepoint already stored in YY */

			}	/* END FOR jj in [1,2] */
		    }		/* END FOR ii in [0,1] */
		}		/* END FOR i */

	      if (pb == 0)
		{
		  pb = 1;
		}
	      else if (pc == 0)
		{
		  pc = 1;
		}



	      fi = (float) pa / (float) pc - (float) pd / (float) pb;
	      if (fi > 180.0F)
		{

		  fi = (pa + pd + pb * 360) / (float) tot;
		  if (fi > 180.0F)
		    fi -= 360.0F;
		}
	      else
		{
		  fi = (pa + pd) / (float) tot;
		}

	      pa = ROUND (fi);
	      if (pa <= -180)
		pa += 360;



	      avv[tp][0] = pa;

	      for (ii = 1; ii < 5; ii++)
		{
		  avv[tp][ii] = avn[ii] / tot;
		  avn[ii] = 0;
		}

	      ct[tp] = tot;
	      gct[tp] = tot;

	      if (tot > match_score)	/* If current TOT > match_score ... */
		match_score = tot;	/*      Keep track of max TOT in match_score */

	      ctt[tp] = 0;	/* Init CTT[TP] to 0 */
	      ctp[tp][0] = tp;	/* Store TP into CTP */

	      for (ii = 0; ii < tp; ii++)
		{
		  int found;
		  int diff;

		  int *avv_tp_ptr = &avv[tp][0];
		  int *avv_ii_ptr = &avv[ii][0];
		  diff = *avv_tp_ptr++ - *avv_ii_ptr++;
		  j = SQUARED (diff);






		  if (j > TXS && j < CTXS)
		    continue;









		  ll = *avv_tp_ptr++ - *avv_ii_ptr++;
		  jj = *avv_tp_ptr++ - *avv_ii_ptr++;
		  kk = *avv_tp_ptr++ - *avv_ii_ptr++;
		  j = *avv_tp_ptr++ - *avv_ii_ptr++;

		  {
		    float tt, ai, dz;

		    tt = (float) (SQUARED (ll) + SQUARED (jj));
		    ai = (float) (SQUARED (j) + SQUARED (kk));

		    fi = (2.0F * TK) * (tt + ai);
		    dz = tt - ai;


		    if (SQUARED (dz) > SQUARED (fi))
		      continue;
		  }



		  if (ll)
		    {

		      if (m1_xyt)
			fi =
			  (180.0F / PI_SINGLE) * atanf ((float) -jj /
							(float) ll);
		      else
			fi =
			  (180.0F / PI_SINGLE) * atanf ((float) jj /
							(float) ll);
		      if (fi < 0.0F)
			{
			  if (ll < 0)
			    fi += 180.5F;
			  else
			    fi -= 0.5F;
			}
		      else
			{
			  if (ll < 0)
			    fi -= 180.5F;
			  else
			    fi += 0.5F;
			}
		      jj = (int) fi;
		      if (jj <= -180)
			jj += 360;
		    }
		  else
		    {

		      if (m1_xyt)
			{
			  if (jj > 0)
			    jj = -90;
			  else
			    jj = 90;
			}
		      else
			{
			  if (jj > 0)
			    jj = 90;
			  else
			    jj = -90;
			}
		    }



		  if (kk)
		    {

		      if (m1_xyt)
			fi =
			  (180.0F / PI_SINGLE) * atanf ((float) -j /
							(float) kk);
		      else
			fi =
			  (180.0F / PI_SINGLE) * atanf ((float) j /
							(float) kk);
		      if (fi < 0.0F)
			{
			  if (kk < 0)
			    fi += 180.5F;
			  else
			    fi -= 0.5F;
			}
		      else
			{
			  if (kk < 0)
			    fi -= 180.5F;
			  else
			    fi += 0.5F;
			}
		      j = (int) fi;
		      if (j <= -180)
			j += 360;
		    }
		  else
		    {

		      if (m1_xyt)
			{
			  if (j > 0)
			    j = -90;
			  else
			    j = 90;
			}
		      else
			{
			  if (j > 0)
			    j = 90;
			  else
			    j = -90;
			}
		    }





		  pa = 0;
		  pb = 0;
		  pc = 0;
		  pd = 0;

		  if (avv[tp][0] < 0)
		    {
		      pd += avv[tp][0];
		      pb++;
		    }
		  else
		    {
		      pa += avv[tp][0];
		      pc++;
		    }

		  if (avv[ii][0] < 0)
		    {
		      pd += avv[ii][0];
		      pb++;
		    }
		  else
		    {
		      pa += avv[ii][0];
		      pc++;
		    }

		  if (pb == 0)
		    {
		      pb = 1;
		    }
		  else if (pc == 0)
		    {
		      pc = 1;
		    }



		  fi = (float) pa / (float) pc - (float) pd / (float) pb;

		  if (fi > 180.0F)
		    {
		      fi = (pa + pd + pb * 360) / 2.0F;
		      if (fi > 180.0F)
			fi -= 360.0F;
		    }
		  else
		    {
		      fi = (pa + pd) / 2.0F;
		    }

		  pb = ROUND (fi);
		  if (pb <= -180)
		    pb += 360;





		  pa = jj - j;
		  pa = IANGLE180 (pa);
		  kk = SQUARED (pb - pa);




		  /* Was: if ( SQUARED(kk) > TXS && kk < CTXS ) : assume typo */
		  if (kk > TXS && kk < CTXS)
		    continue;


		  found = 0;
		  for (kk = 0; kk < 2; kk++)
		    {
		      jj = 0;
		      ll = 0;

		      do
			{
			  while (yy[jj][kk][ii] < yy[ll][kk][tp]
				 && jj < pbz_data->yl[kk][ii])
			    {

			      jj++;
			    }




			  while (yy[jj][kk][ii] > yy[ll][kk][tp]
				 && ll < pbz_data->yl[kk][tp])
			    {

			      ll++;
			    }




			  if (yy[jj][kk][ii] == yy[ll][kk][tp]
			      && jj < pbz_data->yl[kk][ii]
			      && ll < pbz_data->yl[kk][tp])
			    {
			      found = 1;
			      break;
			    }


			}
		      while (jj < pbz_data->yl[kk][ii]
			     && ll < pbz_data->yl[kk][tp]);
		      if (found)
			break;
		    }		/* END for kk */

		  if (!found)
		    {		/* If we didn't find what we were searching for ... */
		      gct[ii] += ct[tp];
		      if (gct[ii] > match_score)
			match_score = gct[ii];
		      ++ctt[ii];
		      ctp[ii][ctt[ii]] = tp;
		    }

		}		/* END for ii in [0,TP-1] prior TP group */

	      tp++;		/* Bump TP counter */


	    }			/* END ELSE if ( tot == MSTR ) */



	  if (qh > QQ_SIZE)
	    {
	      fprintf (errorfp,
		       "%s: WARNING: bz_match_score(): qq[] overflow #3 in bozorth3(); qh-1 is %d [p=%s; g=%s]\n",
		       get_progname (), qh - 1, get_probe_filename (),
		       get_gallery_filename ());
	      return QQ_OVERFLOW_SCORE;
	    }
	  for (i = qh - 1; i > 0; i--)
	    {
	      n = pbz_data->qq[i] - 1;
	      if ((pbz_data->tq[n] - 1) >= 0)
		{
		  pbz_data->rq[pbz_data->tq[n] - 1] = 0;
		  pbz_data->tq[n] = 0;
		  pbz_data->zz[n] = 1000;
		}
	    }

	  for (i = dw - 1; i >= 0; i--)
	    {
	      n = rr[i] - 1;
	      if (pbz_data->tq[n])
		{
		  pbz_data->rq[pbz_data->tq[n] - 1] = 0;
		  pbz_data->tq[n] = 0;
		}
	    }

	  i = 0;
	  j = ww - 1;
	  while (i >= 0 && j >= 0)
	    {
	      if (pbz_data->nn[j] < pbz_data->mm[j])
		{
		  ++pbz_data->nn[j];

		  for (i = ww - 1; i >= 0; i--)
		    {
		      int rt = pbz_data->rx[i];
		      if (rt < 0)
			{
			  rt = -rt;
			  rt--;
			  z = pbz_data->rf[i][pbz_data->nn[i] - 1] - 1;



			  if ((pbz_data->tq[z] != (rt + 1)
			       && pbz_data->tq[z])
			      || (pbz_data->rq[rt] != (z + 1)
				  && pbz_data->rq[rt]))
			    break;


			  pbz_data->tq[z] = rt + 1;
			  pbz_data->rq[rt] = z + 1;
			  rr[i] = z + 1;
			}
		      else
			{
			  rt--;
			  z = pbz_data->cf[i][pbz_data->nn[i] - 1] - 1;


			  if ((pbz_data->tq[rt] != (z + 1)
			       && pbz_data->tq[rt])
			      || (pbz_data->rq[z] != (rt + 1)
				  && pbz_data->rq[z]))
			    break;


			  pbz_data->tq[rt] = z + 1;
			  pbz_data->rq[z] = rt + 1;
			  rr[i] = rt + 1;
			}
		    }		/* END for i */

		  if (i >= 0)
		    {
		      for (z = i + 1; z < ww; z++)
			{
			  n = rr[z] - 1;
			  if (pbz_data->tq[n] - 1 >= 0)
			    {
			      pbz_data->rq[pbz_data->tq[n] - 1] = 0;
			      pbz_data->tq[n] = 0;
			    }
			}
		      j = ww - 1;
		    }

		}
	      else
		{
		  pbz_data->nn[j] = 1;
		  j--;
		}

	    }

	  if (tp > 1999)
	    break;

	  dw = ww;


	}
      while (j >= 0);		/* END while endpoint group remain ... */


      if (tp > 1999)
	break;




      n = pbz_data->qq[0] - 1;
      if (pbz_data->tq[n] - 1 >= 0)
	{
	  pbz_data->rq[pbz_data->tq[n] - 1] = 0;
	  pbz_data->tq[n] = 0;
	}

      for (i = ww - 1; i >= 0; i--)
	{
	  n = pbz_data->rx[i];
	  if (n < 0)
	    {
	      n = -n;
	      pbz_data->rp[n - 1] = 0;
	    }
	  else
	    {
	      pbz_data->cp[n - 1] = 0;
	    }

	}

    }				/* END FOR each edge pair */



  if (match_score < MMSTR)
    {
      //return match_score;
    }

  else
    {
      match_score = bz_final_loop (pbz_data, tp, ct, gct, ctt, ctp, yy);
    }
  free (ctt);
  for (ind = 0; ind < CTP_SIZE_1; ind++)
    {
      free (ctp[ind]);
    }
  free (ctp);
  for (ind = 0; ind < YY_SIZE_1; ind++)
    {
      int ind2;
      for (ind2 = 0; ind2 < YY_SIZE_2; ind2++)
	{
	  free (yy[ind][ind2]);
	}
      free (yy[ind]);
    }
  free (yy);
  return match_score;
}


/***********************************************************************/
/* These globals signficantly used by bz_sift () */
/* Now externally defined in bozorth.h */
/* extern int sc[ SC_SIZE ]; */
/* extern int rq[ RQ_SIZE ]; */
/* extern int tq[ TQ_SIZE ]; */
/* extern int rf[ RF_SIZE_1 ][ RF_SIZE_2 ]; */
/* extern int cf[ CF_SIZE_1 ][ CF_SIZE_2 ]; */
/* extern int zz[ ZZ_SIZE ]; */
/* extern int rx[ RX_SIZE ]; */
/* extern int mm[ MM_SIZE ]; */
/* extern int nn[ NN_SIZE ]; */
/* extern int qq[ QQ_SIZE ]; */
/* extern int rk[ RK_SIZE ]; */
/* extern int cp[ CP_SIZE ]; */
/* extern int rp[ RP_SIZE ]; */
/* extern int y[ Y_SIZE ]; */

void bz_sift (struct bz_data_struct *pbz_data, int *ww,	/* INPUT and OUTPUT; endpoint groups index; *ww may be bumped by one or by two */
	      int kz,		/* INPUT only;       endpoint of lookahead Subject edge */
	      int *qh,		/* INPUT and OUTPUT; the value is an index into qq[] and is stored in zz[]; *qh may be bumped by one */
	      int l,		/* INPUT only;       endpoint of lookahead On-File edge */
	      int kx,		/* INPUT only -- index */
	      int ftt,		/* INPUT only */
	      int *tot,		/* OUTPUT -- counter is incremented by one, sometimes */
	      int *qq_overflow	/* OUTPUT -- flag is set only if qq[] overflows */
  )
{
  int n;
  int t;

/* These now externally defined in bozorth.h */
/* extern FILE * errorfp; */
/* extern char * get_progname( void ); */
/* extern char * get_probe_filename( void ); */
/* extern char * get_gallery_filename( void ); */



  n = pbz_data->tq[kz - 1];	/* Lookup On-File edgepoint stored in TQ at index of endpoint of lookahead Subject edge */
  t = pbz_data->rq[l - 1];	/* Lookup Subject edgepoint stored in RQ at index of endpoint of lookahead On-File edge */

  if (n == 0 && t == 0)
    {


      if (pbz_data->sc[kx - 1] != ftt)
	{
	  pbz_data->y[(*tot)++] = kx;
	  pbz_data->rk[kx - 1] = pbz_data->sc[kx - 1];
	  pbz_data->sc[kx - 1] = ftt;
	}

      if (*qh >= QQ_SIZE)
	{
	  fprintf (errorfp,
		   "%s: ERROR: bz_sift(): qq[] overflow #1; the index [*qh] is %d [p=%s; g=%s]\n",
		   get_progname (), *qh, get_probe_filename (),
		   get_gallery_filename ());
	  *qq_overflow = 1;
	  return;
	}
      pbz_data->qq[*qh] = kz;
      pbz_data->zz[kz - 1] = (*qh)++;


      /* The TQ and RQ locations are set, so set them ... */
      pbz_data->tq[kz - 1] = l;
      pbz_data->rq[l - 1] = kz;

      return;
    }				/* END if ( n == 0 && t == 0 ) */









  if (n == l)
    {

      if (pbz_data->sc[kx - 1] != ftt)
	{
	  if (pbz_data->zz[kx - 1] == 1000)
	    {
	      if (*qh >= QQ_SIZE)
		{
		  fprintf (errorfp,
			   "%s: ERROR: bz_sift(): qq[] overflow #2; the index [*qh] is %d [p=%s; g=%s]\n",
			   get_progname (), *qh, get_probe_filename (),
			   get_gallery_filename ());
		  *qq_overflow = 1;
		  return;
		}
	      pbz_data->qq[*qh] = kz;
	      pbz_data->zz[kz - 1] = (*qh)++;
	    }
	  pbz_data->y[(*tot)++] = kx;
	  pbz_data->rk[kx - 1] = pbz_data->sc[kx - 1];
	  pbz_data->sc[kx - 1] = ftt;
	}

      return;
    }				/* END if ( n == l ) */





  if (*ww >= WWIM)		/* This limits the number of endpoint groups that can be constructed */
    return;


  {
    int b;
    int b_index;
    register int i;
    int notfound;
    int lim;
    register int *lptr;

/* If lookahead Subject endpoint previously assigned to TQ but not paired with lookahead On-File endpoint ... */

    if (n)
      {
	b = pbz_data->cp[kz - 1];
	if (b == 0)
	  {
	    b = ++*ww;
	    b_index = b - 1;
	    pbz_data->cp[kz - 1] = b;
	    pbz_data->cf[b_index][0] = n;
	    pbz_data->mm[b_index] = 1;
	    pbz_data->nn[b_index] = 1;
	    pbz_data->rx[b_index] = kz;

	  }
	else
	  {
	    b_index = b - 1;
	  }

	lim = pbz_data->mm[b_index];
	lptr = &pbz_data->cf[b_index][0];
	notfound = 1;

#ifndef NOVERBOSE
	if (verbose_bozorth)
	  {
	    int *llptr = lptr;
	    printf ("bz_sift(): n: looking for l=%d in [", l);
	    for (i = 0; i < lim; i++)
	      {
		printf (" %d", *llptr++);
	      }
	    printf (" ]\n");
	  }
#endif

	for (i = 0; i < lim; i++)
	  {
	    if (*lptr++ == l)
	      {
		notfound = 0;
		break;
	      }
	  }
	if (notfound)
	  {			/* If lookahead On-File endpoint not in list ... */
	    pbz_data->cf[b_index][i] = l;
	    ++pbz_data->mm[b_index];
	  }
      }				/* END if ( n ) */


/* If lookahead On-File endpoint previously assigned to RQ but not paired with lookahead Subject endpoint... */

    if (t)
      {
	b = pbz_data->rp[l - 1];
	if (b == 0)
	  {
	    b = ++*ww;
	    b_index = b - 1;
	    pbz_data->rp[l - 1] = b;
	    pbz_data->rf[b_index][0] = t;
	    pbz_data->mm[b_index] = 1;
	    pbz_data->nn[b_index] = 1;
	    pbz_data->rx[b_index] = -l;


	  }
	else
	  {
	    b_index = b - 1;
	  }

	lim = pbz_data->mm[b_index];
	lptr = &pbz_data->rf[b_index][0];
	notfound = 1;

#ifndef NOVERBOSE
	if (verbose_bozorth)
	  {
	    int *llptr = lptr;
	    printf ("bz_sift(): t: looking for kz=%d in [", kz);
	    for (i = 0; i < lim; i++)
	      {
		printf (" %d", *llptr++);
	      }
	    printf (" ]\n");
	  }
#endif

	for (i = 0; i < lim; i++)
	  {
	    if (*lptr++ == kz)
	      {
		notfound = 0;
		break;
	      }
	  }
	if (notfound)
	  {			/* If lookahead Subject endpoint not in list ... */
	    pbz_data->rf[b_index][i] = kz;
	    ++pbz_data->mm[b_index];
	  }
      }				/* END if ( t ) */

  }

}

/**************************************************************************/

static int
bz_final_loop (struct bz_data_struct *pbz_data, int tp, int ct[CT_SIZE],
	       int gct[GCT_SIZE], int ctt[CTT_SIZE], int **ctp, int ***yy)
{
  int ii, i, t, b, n, k, j, kk, jj;
  int lim;
  int match_score;

/* This array originally declared global, but moved here */
/* locally because it is only used herein.  The use of   */
/* "static" is required as the array will exceed the     */
/* stack allocation on our local systems otherwise.      */
  int **sct;
  sct = malloc (sizeof (int *) * SCT_SIZE_1);
  int ind;
  for (ind = 0; ind < SCT_SIZE_1; ind++)
    {
      sct[ind] = malloc (sizeof (int) * SCT_SIZE_2);
      int ind2;
      for (ind2 = 0; ind2 < SCT_SIZE_2; ind2++)
	{
	  sct[ind][ind2] = 0;
	}
    }

  match_score = 0;
  for (ii = 0; ii < tp; ii++)
    {				/* For each index up to the current value of TP ... */

      if (match_score >= gct[ii])	/* if next group total not bigger than current match_score.. */
	continue;		/*              skip to next TP index */

      lim = ctt[ii] + 1;
      for (i = 0; i < lim; i++)
	{
	  sct[i][0] = ctp[ii][i];
	}

      t = 0;
      pbz_data->y[0] = lim;
      pbz_data->cp[0] = 1;
      b = 0;
      n = 1;
      do
	{			/* looping until T < 0 ... */
	  if (pbz_data->y[t] - pbz_data->cp[t] > 1)
	    {
	      k = sct[pbz_data->cp[t]][t];
	      j = ctt[k] + 1;
	      for (i = 0; i < j; i++)
		{
		  pbz_data->rp[i] = ctp[k][i];
		}
	      k = 0;
	      kk = pbz_data->cp[t];
	      jj = 0;

	      do
		{
		  while (pbz_data->rp[jj] < sct[kk][t] && jj < j)
		    jj++;
		  while (pbz_data->rp[jj] > sct[kk][t] && kk < pbz_data->y[t])
		    kk++;
		  while (pbz_data->rp[jj] == sct[kk][t]
			 && kk < pbz_data->y[t] && jj < j)
		    {
		      sct[k][t + 1] = sct[kk][t];
		      k++;
		      kk++;
		      jj++;
		    }
		}
	      while (kk < pbz_data->y[t] && jj < j);

	      t++;
	      pbz_data->cp[t] = 1;
	      pbz_data->y[t] = k;
	      b = t;
	      n = 1;
	    }
	  else
	    {
	      int tot = 0;

	      lim = pbz_data->y[t];
	      for (i = n - 1; i < lim; i++)
		{
		  tot += ct[sct[i][t]];
		}

	      for (i = 0; i < b; i++)
		{
		  tot += ct[sct[0][i]];
		}

	      if (tot > match_score)
		{		/* If the current total is larger than the running total ... */
		  match_score = tot;	/*      then set match_score to the new total */
		  for (i = 0; i < b; i++)
		    {
		      pbz_data->rk[i] = sct[0][i];
		    }

		  {
		    int rk_index = b;
		    lim = pbz_data->y[t];
		    for (i = n - 1; i < lim;)
		      {
			pbz_data->rk[rk_index++] = sct[i++][t];
		      }
		  }
		}
	      b = t;
	      t--;
	      if (t >= 0)
		{
		  ++pbz_data->cp[t];
		  n = pbz_data->y[t];
		}
	    }			/* END IF */

	}
      while (t >= 0);

    }				/* END FOR ii */

  for (ind = 0; ind < SCT_SIZE_1; ind++)
    {
      free (sct[ind]);
    }
  free (sct);
  return match_score;

}				/* END bz_final_loop() */
