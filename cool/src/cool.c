/* ------------------------------------------------------------------- */
/*  ACCORDS PLATFORM                                                   */
/*  (C) 2011 by Iain James Marshall (Prologue) <ijm667@hotmail.com>    */
/* --------------------------------------------------------------------*/
/*  This is free software; you can redistribute it and/or modify it    */
/*  under the terms of the GNU Lesser General Public License as        */
/*  published by the Free Software Foundation; either version 2.1 of   */
/*  the License, or (at your option) any later version.                */
/*                                                                     */
/*  This software is distributed in the hope that it will be useful,   */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of     */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   */
/*  Lesser General Public License for more details.                    */
/*                                                                     */
/*  You should have received a copy of the GNU Lesser General Public   */
/*  License along with this software; if not, write to the Free        */
/*  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA */
/*  02110-1301 USA, or see the FSF site: http://www.fsf.org.           */
/* --------------------------------------------------------------------*/
#ifndef	_cool_c	
#define	_cool_c

#include "standard.h"
#include "broker.h"
#include "rest.h"
#include "occi.h"
#include "occiclient.h"
#include "document.h"
#include "cool.h"
#include "cordslang.h"
#include "cb.h"

/* 	----------------------------------------	*/	
/*	Prototype Contract Negotiation Functions	*/
/* 	----------------------------------------	*/
struct	accords_configuration Cool = {
	0,0,
	0,0,0,0,
	(char *) 0,
	(char *) 0, 
	_CORDS_DEFAULT_PUBLISHER,
	_CORDS_DEFAULT_OPERATOR,
	_CORDS_DEFAULT_USER,
	_CORDS_DEFAULT_PASSWORD,
	"http",  80,
	"xmpp",  8000,
	"domain",
	"europe",
	"cool.xml",
	(struct occi_category *) 0,
	(struct occi_category *) 0
	};

public	int	check_debug()		{	return(Cool.debug);		}
public	int	check_verbose()		{	return(Cool.verbose);		}
public	char *	default_publisher()	{	return(Cool.publisher);	}
public	char *	default_operator()	{	return(Cool.operator);		}
public	char *	default_tls()		{	return(Cool.tls);		}
public	char *	default_zone()		{	return(Cool.zone);		}

public	int	failure( int e, char * m1, char * m2 )
{
	if ( e )
	{
		printf("\n*** failure %u",e);
		if ( m1 )
			printf(" : %s",m1);
		if ( m2 )
			printf(" : %s",m2);
		printf(" **`\n");
	}
	return( e );
}

/*	---------------------------------------------------------------	*/  
/*	c o o l _ c o n f i g u r a t i o n				*/
/*	---------------------------------------------------------------	*/  
/*	this function loads cool configuration				*/
/*	from the xml configuration file.				*/
/*	---------------------------------------------------------------	*/  
private	void	cool_configuration()
{
	load_accords_configuration( &Cool, "cool" );
	return;
}

/*	---------------------------------------------------------------	*/  
/*	c o o l _ b a n n e r						*/
/*	---------------------------------------------------------------	*/  
/*	this function will be called to display module identification 	*/
/*	when launched from the command line without any parameters.	*/
/*	---------------------------------------------------------------	*/  
private	int	cool_banner()
{
	printf("\n   CompatibleOne Elasticity Manager : Version 1.0a.0.01");
	printf("\n   Beta Version : 25/07/2012 ");
	printf("\n   Copyright (c) 2012 Iain James Marshall, Prologue");
	printf("\n");
	accords_configuration_options();
	printf("\n\n");
	return(0);
}

/*	--------------------------------------- 	*/
/*	c o o l _ i n i t i a l i s e			*/
/*	--------------------------------------- 	*/
private	struct rest_server * cool_initialise(  void * v,struct rest_server * sptr )
{
	struct	rest_extension * xptr;
	if (!( xptr = rest_add_extension( sptr ) ))
		return((struct rest_server *) 0);
	else
	{
		xptr->net = (struct connection *) 0;
		return( sptr );
	}
}

/*	-------------------------------------------	*/
/*	c o o l _ a u t h o r i s e 			*/
/*	------------------------------------------- 	*/
private	int	cool_authorise(  void * v,struct rest_client * cptr, char * username, char * password )
{
	if ( strcmp( username, Cool.user ) )
		return(0);
	else if ( strcmp( password, Cool.password ) )
		return(0);
	else if (!( cptr->user = allocate_string( username ) ))
		return(0);
	else if (!( cptr->pass = allocate_string( password ) ))
		return(0);
	else	return(1);
}

/*	------------------------------------------- 	*/
/*	c o o l _ e x t e n s i o n 			*/
/*	-------------------------------------------	*/
private	struct rest_extension * cool_extension( void * v,struct rest_server * sptr, struct rest_extension * xptr)
{
	return( xptr );
}

/*	-------------------------------------------	*/
/*	E l a s t i c i t y   M a n a g e m e n t		*/
/*	-------------------------------------------	*/
private	struct	elastic_control Elastic = 
{
	1,	/* floor 			*/
	2, 	/* ceiling			*/
	1,	/* total			*/
	0,	/* strategy: 0= round robin	*/
	0,0,0,0,
	(struct elastic_contract *) 0,
	(struct elastic_contract *) 0,
	(struct elastic_contract *) 0
};

/*	---------------------------------------------------------	*/
/*			l b _ f a i l u r e				*/
/*	---------------------------------------------------------	*/
private	struct rest_response * lb_failure(struct rest_client * cptr,  int status, char * message )
{
	struct	rest_response * aptr;
	if (!( aptr = rest_allocate_response(cptr)))
		return( aptr );
	else	return( rest_html_response( aptr, status, message ) );
}


/*	--------------------------------------------------------------	*/
/*		l i b e r a t e _ e l a s t i c _ c o n t r a c t 		*/
/*	--------------------------------------------------------------	*/
private	struct elastic_contract * liberate_elastic_contract(struct	elastic_contract * eptr)
{
	if ( eptr )
	{
		if ( eptr->service )
			eptr->service = liberate( eptr->service );
		if ( eptr->contract )
			eptr->contract = liberate( eptr->contract );
		eptr = liberate( eptr );
	}
	return((struct elastic_contract *) 0);
}

/*	--------------------------------------------------------------	*/
/*		a l l o c a t e _ e l a s t i c _ c o n t r a c t 		*/
/*	--------------------------------------------------------------	*/
private	struct elastic_contract * allocate_elastic_contract()
{
	struct	elastic_contract * eptr;
	if (!( eptr = (struct elastic_contract *) allocate( sizeof( struct elastic_contract ) ) ))
		return( eptr );
	else	memset( (char *) eptr, 0, sizeof( struct elastic_contract ) );

	if (!( eptr->service = allocate_string( (default_tls()? "https" : "http" ) )))
		return( liberate_elastic_contract( eptr ) );
	else	eptr->port = Cool.restport;
	return( eptr );
}

/*	---------------------------------------------------------	*/
/*		n e x t _ e l a s t i c _ c o n t r a c t 	*/
/*	---------------------------------------------------------	*/
private	struct elastic_contract * next_elastic_contract()
{
	if (!( Elastic.current ))
		return( (Elastic.current = Elastic.first) );
	else if (!( Elastic.current->next ))
		return( (Elastic.current = Elastic.first) );
	else	return( (Elastic.current = Elastic.current->next ) );
}

/*	--------------------------------------------	*/
/*	  u s e _ e l a s t i c _ c o n t r a c t	*/
/*	--------------------------------------------	*/
private	struct elastic_contract * use_elastic_contract( struct elastic_contract * eptr, char * contract)
{
	struct	occi_response * zptr;
	char *	result;

	/* --------------------------------- */
	/* retrieve the contract information */
	/* --------------------------------- */
	if (!( contract ))
		return( liberate_elastic_contract( eptr ) );
	else if (!( zptr = occi_simple_get( contract , _CORDS_CONTRACT_AGENT, default_tls() ) ))
		return( liberate_elastic_contract( eptr ) );

	/* --------------------------- */
	/* the contract hostname value */
	/* --------------------------- */
	else if (!( result = occi_extract_atribut( 
					zptr, Cool.domain, 
					_CORDS_CONTRACT, _CORDS_HOSTNAME )))
		return( liberate_elastic_contract( eptr ) );
	else if (!( eptr->hostname = allocate_string( result ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ----------------------------- */
	/* store the contract identifier */
	/* ----------------------------- */
	if (!( eptr->contract = allocate_string( contract ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ------------------------------- */
	/* append to the list of contracts */
	/* ------------------------------- */
	if (!( eptr->previous = Elastic.last ))
		Elastic.first = eptr;
	else	eptr->previous->next = eptr;
	Elastic.last = eptr;
	Elastic.total++;

	return(eptr);
}

/*	---------------------------------------------------	*/
/*	n e g o t i a t e _ e l a s t i c _ c o n t r a c t	*/
/*	---------------------------------------------------	*/
private	char *	negotiate_elastic_contract(char * node,char * name, char * user, struct cords_placement_criteria * selector)
{
	char *	contract=(char *) 0;
	struct	xml_element * document=(struct xml_element *) 0;
	struct	xml_atribut * aptr;
	if ( check_debug() ) rest_log_message("cool:negotiate_elastic_contract");
	if (!( document = cords_instance_node(
		selector, name, node, _CORDS_CONTRACT_AGENT, default_tls(), (char *) 0, user, user, user) ))
		return( (char *) 0 );
	else if (!( aptr = document_atribut( document, _CORDS_ID ) ))
	{
		document = document_drop( document );
		return((char * ) 0);
	}
	else if (!( contract = allocate_string( aptr->value ) ))
	{
		document = document_drop( document );
		return((char * ) 0);
	}
	else
	{
		document = document_drop( document );
		if ( check_debug() ) rest_log_message("cool:negotiate_elastic_contract:done");
		return(contract);
	}
}

/*	---------------------------------------------	*/
/*	  n e w _ e l a s t i c _ c o n t r a c t	*/
/*	---------------------------------------------	*/
/*	creates a new elastic contract by duplicating	*/
/*	an existing contract and provision instance.	*/
/*	---------------------------------------------	*/
private	struct elastic_contract * new_elastic_contract( struct elastic_contract * eptr, char * contract )
{
	struct	occi_response * zptr=(struct occi_response *) 0;
	struct	occi_response * yptr=(struct occi_response *) 0;
	struct	occi_response * xptr=(struct occi_response *) 0;
	char *	result=(char *) 0;
	char *	profile=(char *) 0;
	char *	provision=(char *) 0;
	char *	account=(char *) 0;
	char *	node=(char *) 0;
	char *	name=(char *) 0;
	struct	cords_placement_criteria selector;
	memset( &selector, 0, sizeof( struct cords_placement_criteria ));

	/* ------------------------------ */
	/* retrieve the CONTRACT instance */
	/* ------------------------------ */
	if (!( contract ))
		return( liberate_elastic_contract( eptr ) );
	else if (!( zptr = occi_simple_get( contract , _CORDS_CONTRACT_AGENT, default_tls() ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ------------------------- */
	/* retrieve the PROFILE name */
	/* ------------------------- */
	else if (!( result = occi_extract_atribut( 
					zptr, Cool.domain, 
					_CORDS_CONTRACT, _CORDS_PROFILE )))
		return( liberate_elastic_contract( eptr ) );
	else if (!( profile = allocate_string( result ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ---------------------------- */
	/* retrieve the NODE identifier */
	/* ---------------------------- */
	else if (!( result = occi_extract_atribut( 
					zptr, Cool.domain, 
					_CORDS_CONTRACT, _CORDS_NODE )))
		return( liberate_elastic_contract( eptr ) );
	else if (!( node = allocate_string( result ) ))
		return( liberate_elastic_contract( eptr ) );

	/* -------------------------------- */
	/* extract the PROVISION identifier */
	/* -------------------------------- */
	else if (!( result = occi_extract_atribut( 
					zptr, Cool.domain, 
					_CORDS_CONTRACT, _CORDS_PROVISION )))
		return( liberate_elastic_contract( eptr ) );
	else if (!( provision = allocate_string( result ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ------------------------------- */
	/* retrieve the PROVISION instance */
	/* ------------------------------- */
	else if (!( yptr = occi_simple_get( provision, _CORDS_CONTRACT_AGENT, default_tls() ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ------------------------------ */
	/* extract the ACCOUNT identifier */
	/* ------------------------------ */
	else if (!( result = occi_extract_atribut( 
					yptr, Cool.domain, 
					profile, _CORDS_ACCOUNT )))
		return( liberate_elastic_contract( eptr ) );
	else if (!( account = allocate_string( result ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ------------------------- */
	/* extract the CONTRACT name */
	/* ------------------------- */
	else if (!( result = occi_extract_atribut( 
					yptr, Cool.domain, 
					profile, _CORDS_NAME )))
		return( liberate_elastic_contract( eptr ) );

	else if (!( name = allocate_string( result ) ))
		return( liberate_elastic_contract( eptr ) );


	else if (!( contract = negotiate_elastic_contract( 
			node, name, account, &selector ) ))
		return( liberate_elastic_contract( eptr ) );

	/* ------------------------------- */
	/* start the new CONTRACT instance */
	/* ------------------------------- */
	cords_invoke_action( contract, "start", _CORDS_SERVICE_AGENT, default_tls() );

	/* ---------------------------- */
	/* add the new ELASTIC CONTRACT */
	/* ---------------------------- */
	return( use_elastic_contract( eptr, contract ) );	
}

/*	--------------------------------------------	*/
/*	  a d d _ e l a s t i c _ c o n t r a c t	*/
/*	--------------------------------------------	*/
private	struct elastic_contract * add_elastic_contract( char * contract, int allocate )
{
	struct	elastic_contract * eptr;
	if (!( eptr = allocate_elastic_contract() ))
		return( eptr );
		
	else if ( allocate )
		return( new_elastic_contract( eptr, contract ) );
	else	return( use_elastic_contract( eptr, contract ) );
}

/*	---------------------------------------------------------	*/
/*		l b _ u p d a t e _ s t a t i s t i c s 			*/
/*	---------------------------------------------------------	*/
private	void	lb_update_statistics()
{	
	int		now=time((long *) 0);
	if ( Elastic.lasthit == now )
		Elastic.hitcount++;
	else
	{
		Elastic.lasthit=now;
		Elastic.hitcount=1;
	}
	if ( Elastic.hitcount > Elastic.maxhit )
		Elastic.maxhit = Elastic.hitcount;

	if ( Elastic.hitcount > Elastic.maxrate )
		if ( Elastic.total < Elastic.ceiling )
			add_elastic_contract( Elastic.first->contract, 1 );
	return;
}

/*	---------------------------------------------------------	*/
/*			l b _ r e d i r e c t					*/
/*	---------------------------------------------------------	*/
private	struct rest_response * lb_redirect( struct rest_client * cptr, struct rest_request * rptr )
{
	struct	rest_header	* hptr;
	struct	rest_response * aptr;
	char		buffer[2048];
	struct	elastic_contract * eptr;

	lb_update_statistics();

	if (!( eptr = next_elastic_contract() )) 
		return( lb_failure(cptr,  500, "Server Failure : No Host" ) );
	else if (!( eptr->hostname ))
		return( lb_failure(cptr,  500, "Server Failure : Bad Host" ) );

	else if (!( aptr = rest_allocate_response(cptr)))
		return( lb_failure(cptr,  500, "Server Failure : Out of Memory" ) );
	else
	{
		sprintf(buffer,"%s://%s:%u",eptr->service,eptr->hostname,eptr->port);
		if (!( hptr = rest_response_header( aptr, _HTTP_LOCATION, buffer )))
		{
			aptr = rest_liberate_response( aptr );
			return( lb_failure(cptr,  500, "Server Failure : Out of Memory" ) );
		}
		else	return( rest_html_response( aptr, _HTTP_MOVED, "Moved" ) );
	}
}

/*	---------------------------------------------------------	*/
/*			l b _ g e t					*/
/*	---------------------------------------------------------	*/
private	struct rest_response * lb_get( 
		void * vptr,
		struct rest_client * cptr, 
		struct rest_request * rptr )
{
	return( lb_redirect(cptr, rptr ) );
}

/*	---------------------------------------------------------	*/
/*			l b _ p u t 					*/
/*	---------------------------------------------------------	*/
private	struct rest_response * lb_put( 
		void * vptr,
		struct rest_client * cptr, 
		struct rest_request * rptr )
{
	return( lb_redirect(cptr, rptr ) );
}

/*	---------------------------------------------------------	*/
/*			l b _ p o s t 					*/
/*	---------------------------------------------------------	*/
private	struct rest_response * lb_post( 
		void * vptr,
		struct rest_client * cptr, 
		struct rest_request * rptr )
{
	return( lb_redirect(cptr, rptr ) );
}

/*	---------------------------------------------------------	*/
/*			l b _ d e l e t e 				*/
/*	---------------------------------------------------------	*/
private	struct rest_response * lb_delete( 
		void * vptr,
		struct rest_client * cptr, 
		struct rest_request * rptr )
{
	return( lb_redirect(cptr, rptr ) );
}

/*	---------------------------------------------------------	*/
/*			l b _ h e a d 					*/
/*	---------------------------------------------------------	*/
private	struct rest_response * lb_head( 
		void * vptr,
		struct rest_client * cptr, 
		struct rest_request * rptr )
{
	return( lb_redirect(cptr, rptr ) );
}

/*	---------------------------------------------------------	*/
/*			l o a d _ b a l a n c e r			*/
/*	---------------------------------------------------------	*/
private	int	load_balancer( char * nptr )
{
	int	status=0;
	struct	rest_interface  Osi = 
	{
		(void *) 0,
		(void *) 0,
		(void *) 0,
		lb_get,
		lb_post,
		lb_put,
		lb_delete,
		lb_head,
		(void *) 0,
		(void *) 0,
		(void *) 0,
		(void *) 0,
		(void *) 0,
		(void *) 0
	};

	if ( Cool.tls )
		if (!( strlen(Cool.tls) ))
			Cool.tls = (char *) 0;

	Osi.authorise = (void *) 0;

	/* ------------------------------------------ */
	/* this parameter now controls thread workers */
	/* ------------------------------------------ */
	rest_thread_control(0);

	/* --------------------------------- */
	/* launch the REST HTTP Server layer */
	/* --------------------------------- */
	return( rest_server(  nptr, Cool.restport, Cool.tls, 0, &Osi ) );
}


/*	--------------------------------------------	*/
/*	c o o l _ o p e r a t i o n 			*/
/*	--------------------------------------------	*/
/*	environment and category preparation		*/
/*	--------------------------------------------	*/
private	int	cool_operation( char * nptr )
{
	char *	eptr;

	if (!( eptr = getenv( "elastic_floor" ) ))
		Elastic.floor = 1;
	else	Elastic.floor = atoi(eptr);

	if (!( eptr = getenv( "elastic_ceiling" ) ))
		Elastic.ceiling = 1;
	else	Elastic.ceiling = atoi(eptr);

	if (!( eptr = getenv( "elastic_strategy" ) ))
		Elastic.floor = 0;
	else	Elastic.floor = atoi(eptr);

	if (!( eptr = getenv( "elastic_contract" ) ))
		return( 118 );
	else if (!( add_elastic_contract( eptr, 0 ) ))
		return( 27 );

	rest_initialise_log( Cool.monitor );

	return( load_balancer( nptr ) );
}

/*	------------------------------------------- 	*/
/*	c o o l _ o p t i o n s			*/
/*	------------------------------------------- 	*/
/*	Command line option analysis			*/
/*	------------------------------------------- 	*/
private	int	cool_options(int argc, char * argv[] )
{
	int	status=0;
	int	argi=0;
	char *	aptr;
	cool_configuration();
	while ( argi < argc )
	{
		if (!( aptr = argv[++argi] ))
			break;
		else if ( *aptr == '-' )
		{
			aptr++;
			switch( *(aptr++) )
			{
			case	'v'	:
				Cool.verbose=1;
				continue;
			case	'd'	:
				Cool.debug = 0xFFFF;
				continue;
			case	'-'	:
				if (!( argi = accords_configuration_option( aptr, argi, argv )))
					break;
				else	continue;
			}
			status = 30;
			break;
		}
		else if (!( status = cool_operation(aptr) ))
			continue;
		else	break;
	}
	return(status);
}

/*	--------------------------------------------	*/
/*	m a i n 					*/
/*	--------------------------------------------	*/
/*	Entry point from the command line		*/
/*	--------------------------------------------	*/
public	int	main(int argc, char * argv[] )
{
	if ( argc == 1 )
		return( cool_banner() );
	else	return( cool_options( argc, argv ) );
}

	/* ---------- */
#endif 	/* _cool_c */
	/* ---------- */

