/* -------------------------------------------------------------------- */
/*  ACCORDS PLATFORM                                                    */
/*  (C) 2011 by Iain James Marshall (Prologue) <ijm667@hotmail.com>     */
/* -------------------------------------------------------------------- */
/* Licensed under the Apache License, Version 2.0 (the "License"); 	*/
/* you may not use this file except in compliance with the License. 	*/
/* You may obtain a copy of the License at 				*/
/*  									*/
/*  http://www.apache.org/licenses/LICENSE-2.0 				*/
/*  									*/
/* Unless required by applicable law or agreed to in writing, software 	*/
/* distributed under the License is distributed on an "AS IS" BASIS, 	*/
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or 	*/
/* implied. 								*/
/* See the License for the specific language governing permissions and 	*/
/* limitations under the License. 					*/
/* -------------------------------------------------------------------- */
#ifndef	_test_cords_broker_c
#define	_test_cords_broker_c

#include "standard.h"
#include "cb.h"
#include "occilogin.h"

struct	cords_broker_config
{
	char *	accept;
	char *	publisher;
	char *	host;
	char *	agent;
	char *	result;
	char *	tls;
	char *	zone;
	char *	operator;
	char *	security;
	int	deployment;
} Cb = 	{
	(char * ) 0,
	_CORDS_DEFAULT_PUBLISHER,
	_CORDS_DEFAULT_PUBLISHER,
	_CORDS_BROKER_AGENT,
	(char *) 0,
	(char *) 0,
	(char *) 0,
	(char *) 0,
	(char *) 0,
	1
	};

private	int	debug=0;
private	int	verbose=0;
public	int	check_debug()		{	return(debug);		}
public	int	check_verbose()		{	return(verbose);	}
public	char *	default_publisher()	{	return(Cb.publisher);	}
public	char *	default_tls()		{	return(Cb.tls);		}
public	char *	default_operator()	{	return(Cb.operator);	}
public	char *	default_zone()		{	return(Cb.zone);	}

public	struct occi_request * cords_account_request( struct occi_client * kptr, char * object, int type );

public	int 	failure( int v, char * mptr, char * aptr )
{
	printf("\ncords: error(%u): %s %s\n",v,(mptr ? mptr : "" ), (aptr ? aptr : "" ) );
	return(v);
}

/*	-----------------------------------------------------	*/
/*		c o r d s _ i n s t a n c e _ p l a n		*/
/*	-----------------------------------------------------	*/
private	int	cords_instance_plan( char * host, char * plan, char * agent, char * result )
{
	struct	occi_response * zptr;
	char	*	sptr;
	initialise_occi_resolver( host, (char *) 0, (char *) 0, (char *) 0 );
	if (!( sptr = occi_unquoted_value( plan )))
		return(500);
	else if (!( zptr =  cords_invoke_action( sptr, _CORDS_INSTANCE, agent, Cb.tls ) ))
		return(501);
	else
	{
		zptr = occi_remove_response( zptr );
		return( 0 );
	}
}

/*	-----------------------------------------------------	*/
/*	   c o r d s _ i n s t a n c e _ a g r e e m e n t	*/
/*	-----------------------------------------------------	*/
private	int	cords_instance_agreement( char * host, char * name, char * sla, char * manifest, char * plan,  char * agent, char * result )
{
	struct	occi_response * zptr;
	char	*	namev;
	char	*	planv;
	char 	*	slav;
	char  	*	manv;
	char 	*	ihost;
	char 	*	aptr;
	int		i;
	int		j;
	FILE	*	h;
	struct	occi_client * kptr;
	struct	occi_request* qptr;
	struct	occi_element* dptr;
	struct	occi_response* yptr;
	char	buffer[1024];

	/* ------------------- */
	/* prepare environment */
	/* ------------------- */
	initialise_occi_resolver( host, (char *) 0, (char *) 0, (char *) 0 );

	/* ------------------- */
	/* validate parameters */
	/* ------------------- */
	if (!( manv = occi_unquoted_value( manifest )))
		return(531);
	else if (!( slav = occi_unquoted_value( sla  )))
		return(532);
	else if (!( planv = occi_unquoted_value( plan )))
		return(533);
	else if (!( namev = occi_unquoted_value( name )))
		return(533);

	/* ------------------------------------------- */
	/* resolve a service category instance manager */
	/* ------------------------------------------- */
	if (!( ihost = occi_resolve_category_provider( _CORDS_SERVICE, agent, default_tls() ) ))
		return(478);

	/* ------------------------------------------ */
	/* url for service category instance creation */ 
	/* ------------------------------------------ */
	else
	{
		sprintf(buffer,"%s/%s/",ihost,_CORDS_SERVICE);
		liberate( ihost );
	}

	/* ------------------------------------------ */
	/* create the SLA controlled service instance */
	/* ------------------------------------------ */
	if (!( kptr = occi_create_client( buffer, agent, default_tls() ) ))
		return(546);
	else if (!( qptr = cords_account_request( kptr, kptr->target->object, _OCCI_NORMAL )))
	{
		kptr = occi_remove_client( kptr );
		return(550);
	}
	else if ((!(dptr=occi_request_element(qptr,"occi.service.plan"  	, planv ) ))
	     ||  (!(dptr=occi_request_element(qptr,"occi.service.manifest"   	, manv  ) ))
	     ||  (!(dptr=occi_request_element(qptr,"occi.service.name"   	, namev ) ))
	     ||  (!(dptr=occi_request_element(qptr,"occi.service.sla"  		, slav 	) )))
	{
		qptr = occi_remove_request( qptr );
		kptr = occi_remove_client( kptr );
		return(551);
	}
	else if (!( yptr = occi_client_post( kptr, qptr ) ))
	{
		qptr = occi_remove_request( qptr );
		kptr = occi_remove_client( kptr );
		return(552);
	}
	else if (!( ihost = occi_extract_location( yptr ) ))
	{
		yptr = occi_remove_response( yptr );
		qptr = occi_remove_request( qptr );
		kptr = occi_remove_client( kptr );
		return(553);
	}
	/* -------------------------------------- */
	/* duplicate the host before the clean up */
	/* -------------------------------------- */
	else if (!( ihost = allocate_string( ihost ) ))
	{
		yptr = occi_remove_response( yptr );
		qptr = occi_remove_request( qptr );
		kptr = occi_remove_client( kptr );
		return(554);
	}
	else
	{
		/* ------------------------------- */
		/* clean up after service creation */
		/* ------------------------------- */
		yptr = occi_remove_response( yptr );
		qptr = occi_remove_request( qptr );
		kptr = occi_remove_client( kptr );
	
		/* -------------------------------- */
		/* start the SLA controlled service */
		/* -------------------------------- */
		rest_add_http_prefix(buffer,1024,ihost);

		if (!( Cb.deployment ))
		{
			for ( j=0,i=0; *(ihost+i) != 0; i++)
				if ( *(ihost+i) == '/' )
					j = (i+1);
			sprintf(buffer,"service/%s",(ihost+j));
			printf("%s\n",buffer);
			if ((h = fopen(buffer,"w")) != (FILE *) 0)
			{
				fprintf(h,"{}\n");
				fclose(h);
			}
			ihost = liberate( ihost );
			return( 0 );
		}
		else 
		{
			ihost = liberate( ihost );
			if (!( zptr =  cords_invoke_action( buffer, _CORDS_START, agent, default_tls() ) ))
				return(503);
			else
			{
				zptr = occi_remove_response( zptr );
				return( 0 );
			}
		}
	}
}

/*	-----------------------------------------------------	*/
/*	   l l _ s l a _ b r o k e r _ o p e r a t i o n	*/
/*	-----------------------------------------------------	*/
/*	this function will provision an instance of service	*/
/*	as described by the input document which will provide 	*/
/*	plan identifier that will be used to resolve to the	*/
/*	manifest description of the service. The operation	*/
/*	can be initated on either a manifest document type or	*/
/*	for the new agreement document type under sla control	*/
/*	-----------------------------------------------------	*/
private	int	ll_sla_broker_operation( char * filename )
{
	struct	occi_response * zptr;
	struct	xml_element * dptr;
	struct	xml_element * eptr;
	struct	xml_element * fptr;
	struct	xml_element * tptr;
	struct	xml_element * xptr;
	struct	xml_atribut * aptr;
	struct	xml_atribut * gptr;
	struct	xml_atribut * mptr;
	struct	xml_atribut * pptr;
	char *	nptr;
	char	nameplan[512];
	int	status;

	/* ------------------- */
	/* validate parameters */
	/* ------------------- */
	if (!(nptr = Cb.result))
		sprintf((nptr=nameplan),"instance_%s",filename);
	if (!( Cb.publisher ))
		return( failure(1,"requires","publication host"));
	else if (!( Cb.agent ))
		return( failure(2,"requires","parser agent name"));
	else if (!( filename ))
		return( failure(3,"requires","cords filename"));

	/* -------------------------- */
	/* process the input document */
	/* -------------------------- */
	else if (!( dptr = document_parse_file( filename ) ))
		return( failure(4,"parse error",filename));

	/* ----------------------------- */
	/* detect manifest document type */
	/* ----------------------------- */
	if (( eptr = document_element( dptr, _CORDS_MANIFEST )) != (struct xml_element *) 0)
	{
		if (!( aptr = document_atribut( eptr, _CORDS_PLAN ) ))
			return( failure(6,"failure resolving plan",filename));
		else if ((status = cords_instance_plan( Cb.publisher, aptr->value, Cb.agent, nptr )) != 0)
			return( failure(status,"failure provisioning plan",aptr->value));
		else	return( 0 );
	}

	/* ------------------------------ */
	/* detect agreement document type */
	/* ------------------------------ */
	else if (!( fptr = document_element( dptr, _CORDS_AGREEMENT ) ))
		return( failure(5,"expected manifest or agreement document",filename));
	else if (!( gptr = document_atribut( fptr, _CORDS_ID ) ))
		return( failure(5,"missing agreement identifier",filename));
	else if (!( tptr = document_element( fptr, _CORDS_TERMS ) ))
		return( failure(5,"missing agreement terms",filename));
	else
	{
		for ( 	eptr = (struct xml_element *) 0,
			xptr = document_element( tptr, _CORDS_TERM );
			xptr != (struct xml_element *) 0;
			tptr = xptr->next )
		{
			if (!( eptr = document_element( xptr, _CORDS_MANIFEST ) ))
				continue;
			else	break;
		}
		if (!( eptr ))
			return( failure(5,"missing manifest element",filename));
		else if (!( mptr = document_atribut( eptr, _CORDS_ID ) ))
			return( failure(5,"missing manifest identifier",filename));
		else if (!( pptr = document_atribut( eptr, _CORDS_ID ) ))
			return( failure(5,"missing manifest name",filename));
		else if (!( aptr = document_atribut( eptr, _CORDS_PLAN ) ))
			return( failure(6,"missing plan identifier",filename));
		else if ((status = cords_instance_agreement( Cb.publisher, pptr->value, gptr->value, mptr->value, aptr->value, Cb.agent, nptr )) != 0)
			return( failure(status,"failure to provision plan",aptr->value));
		else	return( 0 );
	}
}

/*	-----------------------------------------------------	*/
/*	t e s t _ c o r d s _ b r o k e r _ o p e r a t i o n	*/
/*	-----------------------------------------------------	*/
private	int	test_cords_broker_operation( char * filename )
{
	int	status;
	char *	auth;

	if ( Cb.accept )
	{
		occi_client_accept( Cb.accept );
	}

	initialise_occi_resolver( _DEFAULT_PUBLISHER, (char *) 0, (char *) 0, (char *) 0 );

	if (!( auth = login_occi_user( "test-broker","co-system",Cb.agent, Cb.tls ) ))
		return(403);
	else 	(void) occi_client_authentication( auth );

	status = ll_sla_broker_operation( filename );

	(void) logout_occi_user( "test-broker","co-system",Cb.agent, auth, Cb.tls );	

	return( status );
}

/*	-----------------------------------------------------	*/
/*	   t e s t _ c o r d s _ b r o k e r _ c o m a n d 	*/
/*	-----------------------------------------------------	*/
private int	test_cords_broker_command( int	argc, char * argv[] )
{
	int	argi=1;
	char *	aptr;

	while (argi < argc)
	{
		if (!( aptr = argv[argi++] ))
			break;
		else if ( *aptr !=  '-')
			return( test_cords_broker_operation( aptr ) );
		else 
		{
			aptr++;
			if ( *aptr ==  '-')
			{
				aptr++;
				if (!( strcmp( aptr, "verbose" ) ))
				{	verbose=1; continue;	}
				else if (!( strcmp( aptr, "debug" ) ))
				{	debug=1; continue;	}
				else if (!( strcmp( aptr, "tls" ) ))
				{
					Cb.tls = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "publisher" ) ))
				{
					Cb.publisher = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "mime" ) ))
				{
					Cb.accept = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "host" ) ))
				{
					Cb.host = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "zone" ) ))
				{
					Cb.zone = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "account" ) ))
				{
					set_default_account( argv[argi++] );
					continue;
				}
				else if (!( strcmp( aptr, "no-dep" ) ))
				{
					Cb.deployment = 0;
					continue;
				}
				else if (!( strcmp( aptr, "no-deployment" ) ))
				{
					Cb.deployment = 0;
					continue;
				}
				else if (!( strcmp( aptr, "operator" ) ))
				{
					Cb.operator = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "security" ) ))
				{
					Cb.security = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "agent" ) ))
				{
					Cb.agent = argv[argi++];
					continue;
				}
				else if (!( strcmp( aptr, "result" ) ))
				{
					Cb.result = argv[argi++];
					continue;
				}
				else
				{
					printf("incorrect option : %s\n",aptr);
					break;
				}
			
			}
			else
			{
				switch( *(aptr+1) )
				{
				case	'e'	:
					set_xml_echo(1);
					continue;
				case	'd'	:
					debug = 1;
					continue;
				case	'v'	:
					verbose = 1;
					continue;
				case	'h'	:
					Cb.host = (aptr+2);
					continue;
				case	'a'	:
					Cb.agent = (aptr+2);
					continue;
				case	'r'	:
					Cb.result = (aptr+2);
					continue;
				default		:
					printf("incorrect option : %s \n",aptr);
					break;
				}
			}
		}
	}
	return(0);
}

/*	-----------------------------------------------------	*/
/*	   t e s t _ c o r d s _ b r o k e r _ b a n n e r 	*/
/*	-----------------------------------------------------	*/
private	int	test_cords_broker_banner(char * n)
{
	printf("\n   Cords Broker : Version 1.0.b.0.03 ");
	printf("\n   Beta Version 12/09/2013 \n");
	printf("\n   Copyright (c) 2011, 2013 Iain James Marshall, Prologue ");
	printf("\n   Usage : \n");
	printf("\n   --tls  <name>        specify the tls configuration  ");
	printf("\n   --host <host>        specify the publisher hostname ");
	printf("\n   --agent <name>       specify the name of the agent ");
	printf("\n   --accept <type>      specify ACCEPT MIME type ");
	printf("\n   --result <filename>  specify the output plan filename ");
	printf("\n   --account <name>     specify payment account name ");
	printf("\n   --zone <zone>        specify required provisioning zone ");
	printf("\n   --operator <name>    specify required operator name ");
	printf("\n   --security <type>    specify required security level");
	printf("\n   --no-deployment      inhibit automatic provisioning ");
	printf("\n   --verbose            activate verbose messages ");
	printf("\n   --debug              activate debug messages \n");
	printf("\n   Example : \n");
	printf("\n   %s --host http://127.0.0.1:8086 cords.xml \n\n",n);
	return(0);
}

/*	-----------------------------------------------------	*/
/*	     t e s t _ c o r d s _ b r o k e r _ m a i n	*/
/*	-----------------------------------------------------	*/
public	int	main(int argc, char * argv[])
{
	if ( argc == 1 )
		return( test_cords_broker_banner(argv[0]) );
	else	return( test_cords_broker_command( argc, argv ) );
}


#endif


