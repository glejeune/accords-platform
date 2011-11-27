/* ------------------------------------------------------------------------------------	*/
/*				 CompatibleOne Cloudware				*/
/* ------------------------------------------------------------------------------------ */
/*											*/
/* Ce fichier fait partie de ce(tte) oeuvre de Iain James Marshall et est mise a 	*/
/* disposition selon les termes de la licence Creative Commons Paternit� : 		*/
/*											*/
/*			 	Pas d'Utilisation Commerciale 				*/
/*				Pas de Modification 					*/
/*				3.0 non transcrit.					*/
/*											*/
/* ------------------------------------------------------------------------------------ */
/* 			Copyright (c) 2011 Iain James Marshall for Prologue 		*/
/*				   All rights reserved					*/
/* ------------------------------------------------------------------------------------ */
#ifndef _node_c_
#define _node_c_

#include "element.h"

#include "node.h"

/*	------------------------------------------------	*/
/*	l i b e r a t e _ c o r d s _ a p p l i a n c e 	*/
/*	------------------------------------------------	*/
public struct cords_node * liberate_cords_node(struct cords_node * sptr)
{
	if ( sptr )
	{
		if ( sptr->id )
			 sptr->id = liberate(sptr->id);
		if ( sptr->name )
			 sptr->name = liberate(sptr->name);
		if ( sptr->provider )
			 sptr->provider = liberate(sptr->provider);
		if ( sptr->profile )
			 sptr->profile = liberate(sptr->profile);
		if ( sptr->infrastructure )
			 sptr->infrastructure = liberate(sptr->infrastructure);
		if ( sptr->image )
			 sptr->image = liberate(sptr->image);
		sptr = liberate( sptr );
	}
	return((struct cords_node *) 0);

}

/*	------------------------------------------	*/
/*	r e s e t _ c o r d s _ a p p l i a n c e 	*/
/*	------------------------------------------	*/
public struct cords_node * reset_cords_node(struct cords_node * sptr)
{
	if ( sptr )
	{
		sptr->id = (char*) 0;
		sptr->name = (char*) 0;
		sptr->provider = (char*) 0;
		sptr->profile = (char*) 0;
		sptr->infrastructure = (char*) 0;
		sptr->image = (char*) 0;
		sptr->state =  0;
	}
	return(sptr);

}

/*	------------------------------------------------	*/
/*	a l l o c a t e _ c o r d s _ a p p l i a n c e 	*/
/*	------------------------------------------------	*/
public struct cords_node * allocate_cords_node()
{
	struct cords_node * sptr;
	if (!( sptr = allocate( sizeof( struct cords_node ) ) ))
		return( sptr );
	else	return( reset_cords_node(sptr) );
}

/*	------------------------------------------	*/
/*	x m l i n _ c o r d s _ a p p l i a n c e 	*/
/*	------------------------------------------	*/
public int xmlin_cords_node(struct cords_node * sptr,struct xml_element * eptr)
{
	struct xml_element * wptr;
	if (!( eptr )) return(0);
	if (!( sptr )) return(0);
	for ( wptr=eptr->first; wptr != (struct xml_element *) 0; wptr=wptr->next)
	{
		if (!( strcmp(wptr->name,"id") ))
		{
			if ( wptr->value ) { sptr->id = allocate_string(wptr->value); }
		}
		else if (!( strcmp(wptr->name,"name") ))
		{
			if ( wptr->value ) { sptr->name = allocate_string(wptr->value); }
		}
		else if (!( strcmp(wptr->name,"provider") ))
		{
			if ( wptr->value ) { sptr->provider = allocate_string(wptr->value); }
		}
		else if (!( strcmp(wptr->name,"profile") ))
		{
			if ( wptr->value ) { sptr->profile = allocate_string(wptr->value); }
		}
		else if (!( strcmp(wptr->name,"infrastructure") ))
		{
			if ( wptr->value ) { sptr->infrastructure = allocate_string(wptr->value); }
		}
		else if (!( strcmp(wptr->name,"image") ))
		{
			if ( wptr->value ) { sptr->image = allocate_string(wptr->value); }
		}
		else if (!( strcmp(wptr->name,"state") ))
		{
			if ( wptr->value ) { sptr->state = atoi(wptr->value); }
		}
	}
	return(0);

}

/*	--------------------------------------------------	*/
/*	r e s t _ o c c i _ c o r d s _ a p p l i a n c e 	*/
/*	--------------------------------------------------	*/
public int rest_occi_cords_node(FILE * fh,struct cords_node * sptr,char * prefix, char * nptr)
{
	struct xml_element * wptr;
	if (!( sptr )) return(0);
	fprintf(fh,"POST /%s/ HTTP/1.1\r\n",nptr);
	fprintf(fh,"Category: %s; scheme='http://scheme.%s.org/occi/%s#'; class='kind';\r\n",nptr,prefix,prefix);
	fprintf(fh,"X-OCCI-Attribute: %s.%s.id='%s'\r\n",prefix,nptr,(sptr->id?sptr->id:""));
	fprintf(fh,"X-OCCI-Attribute: %s.%s.name='%s'\r\n",prefix,nptr,(sptr->name?sptr->name:""));
	fprintf(fh,"X-OCCI-Attribute: %s.%s.provider='%s'\r\n",prefix,nptr,(sptr->provider?sptr->provider:""));
	fprintf(fh,"X-OCCI-Attribute: %s.%s.profile='%s'\r\n",prefix,nptr,(sptr->profile?sptr->profile:""));
	fprintf(fh,"X-OCCI-Attribute: %s.%s.infrastructure='%s'\r\n",prefix,nptr,(sptr->infrastructure?sptr->infrastructure:""));
	fprintf(fh,"X-OCCI-Attribute: %s.%s.image='%s'\r\n",prefix,nptr,(sptr->image?sptr->image:""));
	fprintf(fh,"X-OCCI-Attribute: %s.%s.state='%u'\r\n",prefix,nptr,sptr->state);
	return(0);

}

#endif	/* _node_c_ */