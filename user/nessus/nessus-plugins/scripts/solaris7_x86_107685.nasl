#
# (C) Tenable Network Security
#
#

if ( ! defined_func("bn_random") ) exit(0);
if(description)
{
 script_id(13225);
 script_version ("$Revision: 1.7 $");
 script_bugtraq_id(6991, 7033, 7230, 8641);
 name["english"] = "Solaris 7 (i386) : 107685-11";
 
 script_name(english:name["english"]);
 
 desc["english"] = "
The remote host is missing Sun Security Patch number 107685-11
( sendmail patch).

You should install this patch for your system to be up-to-date.

Solution : http://sunsolve.sun.com/search/document.do?assetkey=1-21-107685-11-1
Risk factor : High";


 script_description(english:desc["english"]);
 
 summary["english"] = "Check for patch 107685-11"; 
 script_summary(english:summary["english"]);
 
 script_category(ACT_GATHER_INFO);
 
 script_copyright(english:"This script is Copyright (C) 2004 Tenable Network Security");
 family["english"] = "Solaris Local Security Checks";
 script_family(english:family["english"]);
 
 script_dependencies("ssh_get_info.nasl");
 script_require_keys("Host/Solaris/showrev");
 exit(0);
}



include("solaris.inc");

e =  solaris_check_patch(release:"5.7_x86", arch:"i386", patch:"107685-11", obsoleted_by:"", package:"SUNWcsr SUNWnisu SUNWsndmr SUNWsndmu");

if ( e < 0 ) security_hole(0);
else if ( e > 0 )
{
	set_kb_item(name:"BID-6991", value:TRUE);
	set_kb_item(name:"BID-7033", value:TRUE);
	set_kb_item(name:"BID-7230", value:TRUE);
	set_kb_item(name:"BID-8641", value:TRUE);
}
