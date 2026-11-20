#include "custom_steamid.h"
#include "tier1/checksum_md5.h"
#include "tier1/strtools.h"
#include "tier0/dbg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined( USE_SDL )
#include "SDL.h"
#if defined( ANDROID ) || defined( __ANDROID__ )
#include "SDL_system.h"
#endif
#endif

#if defined( _WIN32 )
#include <windows.h>
#include <intrin.h>
#elif defined( ANDROID ) || defined( __ANDROID__ )
#include <jni.h>
#include <sys/system_properties.h>
#include <unistd.h>
#else
#include <unistd.h>
#include <sys/utsname.h>
#endif

static char s_szDeviceUUID[64] = { 0 };

static void FormatHashAsUUID( const char *pszInput, char *pszOut, size_t nMaxLen )
{
	MD5Context_t ctx;
	unsigned char digest[16];
	memset( &ctx, 0, sizeof( ctx ) );
	MD5Init( &ctx );
	MD5Update( &ctx, (const unsigned char *)"SA_DEVID_V2:", 12 );
	if ( pszInput && pszInput[0] )
	{
		MD5Update( &ctx, (const unsigned char *)pszInput, V_strlen( pszInput ) );
	}
	MD5Final( digest, &ctx );

	// Set RFC-4122 variant/version bits for a clean deterministic UUID format
	digest[6] = ( digest[6] & 0x0F ) | 0x50;
	digest[8] = ( digest[8] & 0x3F ) | 0x80;

	V_snprintf( pszOut, nMaxLen,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		digest[0], digest[1], digest[2], digest[3],
		digest[4], digest[5],
		digest[6], digest[7],
		digest[8], digest[9],
		digest[10], digest[11], digest[12], digest[13], digest[14], digest[15] );
}

#if !defined( _WIN32 )
static bool ReadTrimmedLineFromFile( const char *pszFilePath, char *pszOut, size_t nMaxLen )
{
	if ( !pszFilePath || !pszFilePath[0] || !pszOut || nMaxLen == 0 )
		return false;

	pszOut[0] = '\0';
	FILE *f = fopen( pszFilePath, "r" );
	if ( !f )
		return false;

	char szBuffer[256] = { 0 };
	if ( fgets( szBuffer, sizeof( szBuffer ) - 1, f ) == NULL )
	{
		fclose( f );
		return false;
	}
	fclose( f );

	char *p = szBuffer;
	while ( *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n' )
		p++;

	size_t len = V_strlen( p );
	while ( len > 0 && ( p[len - 1] == ' ' || p[len - 1] == '\t' || p[len - 1] == '\r' || p[len - 1] == '\n' ) )
	{
		p[len - 1] = '\0';
		len--;
	}

	if ( len == 0 )
		return false;

	V_strncpy( pszOut, p, nMaxLen );
	return true;
}
#endif

#if defined( ANDROID ) || defined( __ANDROID__ )

static bool GetAndroidSecureIdJNI( char *pszOut, size_t nMaxLen )
{
	pszOut[0] = '\0';
#if defined( USE_SDL )
	JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	if ( !env )
		return false;

	jobject activity = (jobject)SDL_AndroidGetActivity();
	if ( !activity )
		return false;

	bool bSuccess = false;
	jclass clsActivity = env->GetObjectClass( activity );
	jobject contentResolver = NULL;
	jclass clsSecure = NULL;
	jstring jKey = NULL;
	jstring jAndroidId = NULL;

	if ( clsActivity )
	{
		jmethodID midGetResolver = env->GetMethodID( clsActivity, "getContentResolver", "()Landroid/content/ContentResolver;" );
		if ( midGetResolver && !env->ExceptionCheck() )
		{
			contentResolver = env->CallObjectMethod( activity, midGetResolver );
		}
	}

	if ( env->ExceptionCheck() )
		env->ExceptionClear();

	if ( contentResolver )
	{
		clsSecure = env->FindClass( "android/provider/Settings$Secure" );
		if ( clsSecure && !env->ExceptionCheck() )
		{
			jmethodID midGetString = env->GetStaticMethodID( clsSecure, "getString",
				"(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;" );
			if ( midGetString && !env->ExceptionCheck() )
			{
				jKey = env->NewStringUTF( "android_id" );
				if ( jKey )
				{
					jAndroidId = (jstring)env->CallStaticObjectMethod( clsSecure, midGetString, contentResolver, jKey );
				}
			}
		}
	}

	if ( env->ExceptionCheck() )
		env->ExceptionClear();

	if ( jAndroidId )
	{
		const char *pszChars = env->GetStringUTFChars( jAndroidId, NULL );
		if ( pszChars )
		{
			// Reject empty or legacy emulator default bug value ("9774d56d682e549c")
			if ( V_strlen( pszChars ) >= 8 && V_stricmp( pszChars, "9774d56d682e549c" ) != 0 )
			{
				V_strncpy( pszOut, pszChars, nMaxLen );
				bSuccess = true;
			}
			env->ReleaseStringUTFChars( jAndroidId, pszChars );
		}
	}

	if ( jAndroidId ) env->DeleteLocalRef( jAndroidId );
	if ( jKey ) env->DeleteLocalRef( jKey );
	if ( clsSecure ) env->DeleteLocalRef( clsSecure );
	if ( contentResolver ) env->DeleteLocalRef( contentResolver );
	if ( clsActivity ) env->DeleteLocalRef( clsActivity );
	env->DeleteLocalRef( activity );

	return bSuccess;
#else
	return false;
#endif
}

static bool GetAndroidWidevineIdJNI( char *pszOut, size_t nMaxLen )
{
	pszOut[0] = '\0';
#if defined( USE_SDL )
	JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
	if ( !env )
		return false;

	bool bSuccess = false;
	jclass clsUUID = NULL;
	jobject objUUID = NULL;
	jclass clsMediaDrm = NULL;
	jobject objMediaDrm = NULL;
	jstring jPropName = NULL;
	jbyteArray jIdBytes = NULL;

	clsUUID = env->FindClass( "java/util/UUID" );
	if ( clsUUID && !env->ExceptionCheck() )
	{
		jmethodID midUUIDInit = env->GetMethodID( clsUUID, "<init>", "(JJ)V" );
		if ( midUUIDInit && !env->ExceptionCheck() )
		{
			// Widevine DRM scheme UUID: edef8ba9-79d6-4ace-a3c8-27dcd51d21ed
			objUUID = env->NewObject( clsUUID, midUUIDInit,
				(jlong)0xEDEF8BA979D64ACEULL, (jlong)0xA3C827DCD51D21EDULL );
		}
	}

	if ( env->ExceptionCheck() )
		env->ExceptionClear();

	if ( objUUID )
	{
		clsMediaDrm = env->FindClass( "android/media/MediaDrm" );
		if ( clsMediaDrm && !env->ExceptionCheck() )
		{
			jmethodID midDrmInit = env->GetMethodID( clsMediaDrm, "<init>", "(Ljava/util/UUID;)V" );
			if ( midDrmInit && !env->ExceptionCheck() )
			{
				objMediaDrm = env->NewObject( clsMediaDrm, midDrmInit, objUUID );
			}
		}
	}

	if ( env->ExceptionCheck() )
		env->ExceptionClear();

	if ( objMediaDrm && clsMediaDrm )
	{
		jmethodID midGetProp = env->GetMethodID( clsMediaDrm, "getPropertyByteArray", "(Ljava/lang/String;)[B" );
		if ( midGetProp && !env->ExceptionCheck() )
		{
			jPropName = env->NewStringUTF( "deviceUniqueId" );
			if ( jPropName )
			{
				jIdBytes = (jbyteArray)env->CallObjectMethod( objMediaDrm, midGetProp, jPropName );
			}
		}

		if ( env->ExceptionCheck() )
			env->ExceptionClear();

		if ( jIdBytes )
		{
			jsize nLen = env->GetArrayLength( jIdBytes );
			if ( nLen >= 4 )
			{
				jbyte *pBytes = env->GetByteArrayElements( jIdBytes, NULL );
				if ( pBytes )
				{
					size_t nOutPos = 0;
					for ( jsize i = 0; i < nLen && ( nOutPos + 2 ) < nMaxLen; ++i )
					{
						V_snprintf( pszOut + nOutPos, nMaxLen - nOutPos, "%02x", (unsigned char)pBytes[i] );
						nOutPos += 2;
					}
					env->ReleaseByteArrayElements( jIdBytes, pBytes, JNI_ABORT );
					bSuccess = ( nOutPos >= 8 );
				}
			}
		}

		// Close MediaDrm cleanly (close() on API 28+, fallback to release() on older Android)
		jmethodID midClose = env->GetMethodID( clsMediaDrm, "close", "()V" );
		if ( env->ExceptionCheck() )
		{
			env->ExceptionClear();
			midClose = env->GetMethodID( clsMediaDrm, "release", "()V" );
			if ( env->ExceptionCheck() )
				env->ExceptionClear();
		}
		if ( midClose )
		{
			env->CallVoidMethod( objMediaDrm, midClose );
			if ( env->ExceptionCheck() )
				env->ExceptionClear();
		}
	}

	if ( jIdBytes ) env->DeleteLocalRef( jIdBytes );
	if ( jPropName ) env->DeleteLocalRef( jPropName );
	if ( objMediaDrm ) env->DeleteLocalRef( objMediaDrm );
	if ( clsMediaDrm ) env->DeleteLocalRef( clsMediaDrm );
	if ( objUUID ) env->DeleteLocalRef( objUUID );
	if ( clsUUID ) env->DeleteLocalRef( clsUUID );

	return bSuccess;
#else
	return false;
#endif
}

static void BuildAndroidHardwareFingerprint( char *pszOut, size_t nMaxLen )
{
	pszOut[0] = '\0';

	// 1. Primary: Settings.Secure.ANDROID_ID (zero permissions, survives uninstall/reinstall & clear data)
	char szAndroidId[128] = { 0 };
	if ( GetAndroidSecureIdJNI( szAndroidId, sizeof( szAndroidId ) ) )
	{
		V_snprintf( pszOut, nMaxLen, "AND_AID|%s", szAndroidId );
		return;
	}

	// 2. Fallback: Widevine MediaDrm hardware deviceUniqueId
	char szWidevineId[256] = { 0 };
	if ( GetAndroidWidevineIdJNI( szWidevineId, sizeof( szWidevineId ) ) )
	{
		V_snprintf( pszOut, nMaxLen, "AND_DRM|%s", szWidevineId );
		return;
	}

	// 3. Last-resort deterministic hardware properties (no random UUID, no .device_id files)
	static const char * const s_PropNames[] =
	{
		"ro.serialno",
		"ro.boot.serialno",
		"ro.product.brand",
		"ro.product.manufacturer",
		"ro.product.model",
		"ro.product.device",
		"ro.hardware",
		"ro.board.platform",
		"ro.build.fingerprint",
	};

	V_strncpy( pszOut, "AND_HW", nMaxLen );
	for ( size_t i = 0; i < sizeof( s_PropNames ) / sizeof( s_PropNames[0] ); ++i )
	{
		char szPropVal[PROP_VALUE_MAX] = { 0 };
		if ( __system_property_get( s_PropNames[i], szPropVal ) > 0 && V_stricmp( szPropVal, "unknown" ) != 0 )
		{
			V_strncat( pszOut, "|", nMaxLen );
			V_strncat( pszOut, szPropVal, nMaxLen );
		}
	}

	char szSocSerial[128] = { 0 };
	if ( ReadTrimmedLineFromFile( "/sys/devices/soc0/serial_number", szSocSerial, sizeof( szSocSerial ) ) )
	{
		V_strncat( pszOut, "|SOC:", nMaxLen );
		V_strncat( pszOut, szSocSerial, nMaxLen );
	}
}

#elif defined( _WIN32 )

static bool ReadRegStringValue( HKEY hRoot, const char *pszSubKey, const char *pszValueName, char *pszOut, size_t nMaxLen )
{
	pszOut[0] = '\0';
	HKEY hKey = NULL;
	if ( RegOpenKeyExA( hRoot, pszSubKey, 0, KEY_READ | KEY_WOW64_64KEY, &hKey ) != ERROR_SUCCESS &&
	     RegOpenKeyExA( hRoot, pszSubKey, 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
	{
		return false;
	}

	char szBuf[256] = { 0 };
	DWORD dwSize = sizeof( szBuf ) - 1;
	DWORD dwType = 0;
	LONG lRes = RegQueryValueExA( hKey, pszValueName, NULL, &dwType, (LPBYTE)szBuf, &dwSize );
	RegCloseKey( hKey );

	if ( lRes != ERROR_SUCCESS || ( dwType != REG_SZ && dwType != REG_EXPAND_SZ ) || dwSize <= 1 )
		return false;

	szBuf[sizeof( szBuf ) - 1] = '\0';
	V_strncpy( pszOut, szBuf, nMaxLen );
	return pszOut[0] != '\0';
}

static void BuildWindowsHardwareFingerprint( char *pszOut, size_t nMaxLen )
{
	V_strncpy( pszOut, "WIN", nMaxLen );

	// 1. OS installation MachineGuid (persists across game reinstalls & AppData deletion)
	char szVal[256] = { 0 };
	if ( ReadRegStringValue( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Cryptography", "MachineGuid", szVal, sizeof( szVal ) ) )
	{
		V_strncat( pszOut, "|MG:", nMaxLen );
		V_strncat( pszOut, szVal, nMaxLen );
	}

	// 2. Windows Hardware Profile GUID
	if ( ReadRegStringValue( HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware Profiles\\0001", "HwProfileGuid", szVal, sizeof( szVal ) ) )
	{
		V_strncat( pszOut, "|HP:", nMaxLen );
		V_strncat( pszOut, szVal, nMaxLen );
	}

	// 3. System drive volume serial number
	char szWinDir[MAX_PATH] = { 0 };
	char szRoot[8] = "C:\\";
	if ( GetSystemWindowsDirectoryA( szWinDir, sizeof( szWinDir ) ) >= 3 && szWinDir[1] == ':' )
	{
		szRoot[0] = szWinDir[0];
		szRoot[1] = ':';
		szRoot[2] = '\\';
		szRoot[3] = '\0';
	}
	DWORD dwVolSerial = 0;
	if ( GetVolumeInformationA( szRoot, NULL, 0, &dwVolSerial, NULL, NULL, NULL, 0 ) && dwVolSerial != 0 )
	{
		char szSerial[32];
		V_snprintf( szSerial, sizeof( szSerial ), "|VS:%08X", (unsigned int)dwVolSerial );
		V_strncat( pszOut, szSerial, nMaxLen );
	}

	// 4. Core-invariant CPU vendor + signature + brand string
	int cpuInfo[4] = { 0 };
	__cpuid( cpuInfo, 0 );
	char szVendor[16];
	memcpy( szVendor + 0, &cpuInfo[1], 4 );
	memcpy( szVendor + 4, &cpuInfo[3], 4 );
	memcpy( szVendor + 8, &cpuInfo[2], 4 );
	szVendor[12] = '\0';

	int sigInfo[4] = { 0 };
	__cpuid( sigInfo, 1 );
	char szCpuSig[64];
	V_snprintf( szCpuSig, sizeof( szCpuSig ), "|CPU:%s:%08X", szVendor, (unsigned int)sigInfo[0] );
	V_strncat( pszOut, szCpuSig, nMaxLen );

	int extInfo[4] = { 0 };
	__cpuid( extInfo, (int)0x80000000 );
	if ( (unsigned int)extInfo[0] >= 0x80000004 )
	{
		char szBrand[64] = { 0 };
		__cpuid( (int *)( szBrand + 0 ), (int)0x80000002 );
		__cpuid( (int *)( szBrand + 16 ), (int)0x80000003 );
		__cpuid( (int *)( szBrand + 32 ), (int)0x80000004 );
		szBrand[48] = '\0';
		V_strncat( pszOut, ":", nMaxLen );
		V_strncat( pszOut, szBrand, nMaxLen );
	}

	// 5. SMBIOS / Motherboard info
	if ( ReadRegStringValue( HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\BIOS", "BaseBoardProduct", szVal, sizeof( szVal ) ) )
	{
		V_strncat( pszOut, "|MB:", nMaxLen );
		V_strncat( pszOut, szVal, nMaxLen );
	}
}

#else

static void BuildPosixHardwareFingerprint( char *pszOut, size_t nMaxLen )
{
	V_strncpy( pszOut, "POSIX", nMaxLen );

	char szVal[256] = { 0 };
	if ( ReadTrimmedLineFromFile( "/etc/machine-id", szVal, sizeof( szVal ) ) ||
	     ReadTrimmedLineFromFile( "/var/lib/dbus/machine-id", szVal, sizeof( szVal ) ) )
	{
		V_strncat( pszOut, "|MID:", nMaxLen );
		V_strncat( pszOut, szVal, nMaxLen );
	}

	if ( ReadTrimmedLineFromFile( "/sys/class/dmi/id/product_uuid", szVal, sizeof( szVal ) ) ||
	     ReadTrimmedLineFromFile( "/sys/devices/virtual/dmi/id/product_uuid", szVal, sizeof( szVal ) ) )
	{
		V_strncat( pszOut, "|DMI:", nMaxLen );
		V_strncat( pszOut, szVal, nMaxLen );
	}

	struct utsname uts;
	if ( uname( &uts ) == 0 )
	{
		V_strncat( pszOut, "|U:", nMaxLen );
		V_strncat( pszOut, uts.sysname, nMaxLen );
		V_strncat( pszOut, ":", nMaxLen );
		V_strncat( pszOut, uts.nodename, nMaxLen );
		V_strncat( pszOut, ":", nMaxLen );
		V_strncat( pszOut, uts.machine, nMaxLen );
	}
}

#endif

const char *GetDeviceUUID()
{
	if ( s_szDeviceUUID[0] != '\0' )
		return s_szDeviceUUID;

	char szFingerprint[1024] = { 0 };

#if defined( ANDROID ) || defined( __ANDROID__ )
	BuildAndroidHardwareFingerprint( szFingerprint, sizeof( szFingerprint ) );
#elif defined( _WIN32 )
	BuildWindowsHardwareFingerprint( szFingerprint, sizeof( szFingerprint ) );
#else
	BuildPosixHardwareFingerprint( szFingerprint, sizeof( szFingerprint ) );
#endif

	FormatHashAsUUID( szFingerprint, s_szDeviceUUID, sizeof( s_szDeviceUUID ) );
	return s_szDeviceUUID;
}

CSteamID GenerateSteamIDFromUUID( const char *pszUUID )
{
	if ( !pszUUID || !pszUUID[0] )
		return CSteamID();

	MD5Context_t ctx;
	unsigned char digest[16];
	memset( &ctx, 0, sizeof(ctx) );
	MD5Init( &ctx );
	MD5Update( &ctx, (const unsigned char *)pszUUID, V_strlen(pszUUID) );
	MD5Final( digest, &ctx );

	uint32 rawID = *(uint32 *)digest;
	uint32 accountID = ( rawID & 0x7FFFFFFF );
	if ( accountID == 0 )
		accountID = 1;

	return CSteamID( accountID, k_unSteamUserDesktopInstance, k_EUniversePublic, k_EAccountTypeIndividual );
}

CSteamID GenerateSteamIDFromAddress( const netadr_t &adr )
{
	char szBuf[64];
	V_snprintf( szBuf, sizeof(szBuf), "ADDR_%s", adr.ToString( true ) );
	return GenerateSteamIDFromUUID( szBuf );
}

CSteamID GetLocalDeviceSteamID()
{
	return GenerateSteamIDFromUUID( GetDeviceUUID() );
}
