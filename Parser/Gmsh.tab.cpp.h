typedef union {
  char    *c;
  int      i;
  double   d;
  double   v[5];
  Shape    s;
  List_T  *l;
} YYSTYPE;
#define	tDOUBLE	258
#define	tSTRING	259
#define	tBIGSTR	260
#define	tEND	261
#define	tAFFECT	262
#define	tDOTS	263
#define	tPi	264
#define	tExp	265
#define	tLog	266
#define	tLog10	267
#define	tSqrt	268
#define	tSin	269
#define	tAsin	270
#define	tCos	271
#define	tAcos	272
#define	tTan	273
#define	tAtan	274
#define	tAtan2	275
#define	tSinh	276
#define	tCosh	277
#define	tTanh	278
#define	tFabs	279
#define	tFloor	280
#define	tCeil	281
#define	tFmod	282
#define	tModulo	283
#define	tHypot	284
#define	tPoint	285
#define	tCircle	286
#define	tEllipsis	287
#define	tLine	288
#define	tSurface	289
#define	tSpline	290
#define	tVolume	291
#define	tCharacteristic	292
#define	tLength	293
#define	tParametric	294
#define	tElliptic	295
#define	tPlane	296
#define	tRuled	297
#define	tTransfinite	298
#define	tComplex	299
#define	tPhysical	300
#define	tUsing	301
#define	tPower	302
#define	tBump	303
#define	tProgression	304
#define	tAssociation	305
#define	tRotate	306
#define	tTranslate	307
#define	tSymmetry	308
#define	tDilate	309
#define	tExtrude	310
#define	tDuplicata	311
#define	tLoop	312
#define	tInclude	313
#define	tRecombine	314
#define	tDelete	315
#define	tCoherence	316
#define	tView	317
#define	tOffset	318
#define	tAttractor	319
#define	tLayers	320
#define	tScalarTetrahedron	321
#define	tVectorTetrahedron	322
#define	tTensorTetrahedron	323
#define	tScalarTriangle	324
#define	tVectorTriangle	325
#define	tTensorTriangle	326
#define	tScalarLine	327
#define	tVectorLine	328
#define	tTensorLine	329
#define	tScalarPoint	330
#define	tVectorPoint	331
#define	tTensorPoint	332
#define	tBSpline	333
#define	tNurbs	334
#define	tOrder	335
#define	tWith	336
#define	tBounds	337
#define	tKnots	338
#define	tColor	339
#define	tGeneral	340
#define	tGeometry	341
#define	tMesh	342
#define	tClip	343
#define	tB_SPLINE_SURFACE_WITH_KNOTS	344
#define	tB_SPLINE_CURVE_WITH_KNOTS	345
#define	tCARTESIAN_POINT	346
#define	tTRUE	347
#define	tFALSE	348
#define	tUNSPECIFIED	349
#define	tU	350
#define	tV	351
#define	tEDGE_CURVE	352
#define	tVERTEX_POINT	353
#define	tORIENTED_EDGE	354
#define	tPLANE	355
#define	tFACE_OUTER_BOUND	356
#define	tEDGE_LOOP	357
#define	tADVANCED_FACE	358
#define	tVECTOR	359
#define	tDIRECTION	360
#define	tAXIS2_PLACEMENT_3D	361
#define	tISO	362
#define	tENDISO	363
#define	tENDSEC	364
#define	tDATA	365
#define	tHEADER	366
#define	tFILE_DESCRIPTION	367
#define	tFILE_SCHEMA	368
#define	tFILE_NAME	369
#define	tMANIFOLD_SOLID_BREP	370
#define	tCLOSED_SHELL	371
#define	tADVANCED_BREP_SHAPE_REPRESENTATION	372
#define	tFACE_BOUND	373
#define	tCYLINDRICAL_SURFACE	374
#define	tCONICAL_SURFACE	375
#define	tCIRCLE	376
#define	tTRIMMED_CURVE	377
#define	tGEOMETRIC_SET	378
#define	tCOMPOSITE_CURVE_SEGMENT	379
#define	tCONTINUOUS	380
#define	tCOMPOSITE_CURVE	381
#define	tTOROIDAL_SURFACE	382
#define	tPRODUCT_DEFINITION	383
#define	tPRODUCT_DEFINITION_SHAPE	384
#define	tSHAPE_DEFINITION_REPRESENTATION	385
#define	tELLIPSE	386
#define	tTrimmed	387
#define	tSolid	388
#define	tEndSolid	389
#define	tVertex	390
#define	tFacet	391
#define	tNormal	392
#define	tOuter	393
#define	tLoopSTL	394
#define	tEndLoop	395
#define	tEndFacet	396
#define	UMINUS	397


extern YYSTYPE yylval;
