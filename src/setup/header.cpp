/*
 * Copyright (C) 2011-2013 Daniel Scharrer
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the author(s) be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include "setup/header.hpp"

#include <cstdio>
#include <cstring>

#include <boost/static_assert.hpp>

#include "setup/version.hpp"
#include "util/encoding.hpp"
#include "util/load.hpp"
#include "util/storedenum.hpp"

namespace setup {

namespace {

STORED_ENUM_MAP(stored_install_verbosity, header::NormalInstallMode,
	header::NormalInstallMode,
	header::SilentInstallMode,
	header::VerySilentInstallMode
);

STORED_ENUM_MAP(stored_log_mode, header::AppendLog,
	header::AppendLog,
	header::NewLog,
	header::OverwriteLog
);

STORED_ENUM_MAP(stored_setup_style, header::ClassicStyle,
	header::ClassicStyle,
	header::ModernStyle
);

STORED_ENUM_MAP(stored_bool_auto_no_yes, header::Auto,
	header::Auto,
	header::No,
	header::Yes
);

// pre- 5.3.7
STORED_ENUM_MAP(stored_privileges_0, header::NoPrivileges,
	header::NoPrivileges,
	header::PowerUserPrivileges,
	header::AdminPriviliges,
);

// post- 5.3.7
STORED_ENUM_MAP(stored_privileges_1, header::NoPrivileges,
	header::NoPrivileges,
	header::PowerUserPrivileges,
	header::AdminPriviliges,
	header::LowestPrivileges
);

STORED_ENUM_MAP(stored_bool_yes_no_auto, header::Yes,
	header::Yes,
	header::No,
	header::Auto
);

STORED_ENUM_MAP(stored_language_detection_method, header::UILanguage,
	header::UILanguage,
	header::LocaleLanguage,
	header::NoLanguageDetection
);

STORED_FLAGS_MAP(stored_architectures,
	header::ArchitectureUnknown,
	header::X86,
	header::Amd64,
	header::IA64
);

// pre-4.2.5
STORED_ENUM_MAP(stored_compression_method_0, stream::UnknownCompression,
	stream::Zlib,
	stream::BZip2,
	stream::LZMA1
);

// 4.2.5
STORED_ENUM_MAP(stored_compression_method_1, stream::UnknownCompression,
	stream::Stored,
	stream::BZip2,
	stream::LZMA1
);

// [4.2.6 5.3.9)
STORED_ENUM_MAP(stored_compression_method_2, stream::UnknownCompression,
	stream::Stored,
	stream::Zlib,
	stream::BZip2,
	stream::LZMA1
);

// 5.3.9+
STORED_ENUM_MAP(stored_compression_method_3, stream::UnknownCompression,
	stream::Stored,
	stream::Zlib,
	stream::BZip2,
	stream::LZMA1,
	stream::LZMA2
);

} // anonymous namespace

void header::load(std::istream & is, const version & version) {
	
	options = 0;
	
	if(version < INNO_VERSION(1, 3, 21)) {
		(void)util::load<boost::uint32_t>(is); // uncompressed size of the setup header
	}
	
	is >> util::encoded_string(app_name, version.codepage());
	is >> util::encoded_string(app_versioned_name, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> util::encoded_string(app_id, version.codepage());
	}
	is >> util::encoded_string(app_copyright, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> util::encoded_string(app_publisher, version.codepage());
		is >> util::encoded_string(app_publisher_url, version.codepage());
	} else {
		app_publisher.clear(), app_publisher_url.clear();
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		is >> util::encoded_string(app_support_phone, version.codepage());
	} else {
		app_support_phone.clear();
	}
	if(version >= INNO_VERSION(1, 3, 21)) {
		is >> util::encoded_string(app_support_url, version.codepage());
		is >> util::encoded_string(app_updates_url, version.codepage());
		is >> util::encoded_string(app_version, version.codepage());
	} else {
		app_support_url.clear(), app_updates_url.clear(), app_version.clear();
	}
	is >> util::encoded_string(default_dir_name, version.codepage());
	is >> util::encoded_string(default_group_name, version.codepage());
	if(version < INNO_VERSION(3, 0, 0)) {
		is >> util::ansi_string(uninstall_icon_name);
	} else {
		uninstall_icon_name.clear();
	}
	is >> util::encoded_string(base_filename, version.codepage());
	if(version >= INNO_VERSION(1, 3, 21)) {
		if(version < INNO_VERSION(5, 2, 5)) {
			is >> util::ansi_string(license_text);
			is >> util::ansi_string(info_before);
			is >> util::ansi_string(info_after);
		}
		is >> util::encoded_string(uninstall_files_dir, version.codepage());
		is >> util::encoded_string(uninstall_name, version.codepage());
		is >> util::encoded_string(uninstall_icon, version.codepage());
		is >> util::encoded_string(app_mutex, version.codepage());
	} else {
		license_text.clear(), info_before.clear(), info_after.clear();
		uninstall_files_dir.clear(), uninstall_name.clear();
		uninstall_icon.clear(), app_mutex.clear();
	}
	if(version >= INNO_VERSION(3, 0, 0)) {
		is >> util::encoded_string(default_user_name, version.codepage());
		is >> util::encoded_string(default_user_organisation, version.codepage());
	} else {
		default_user_name.clear(), default_user_organisation.clear();
	}
	if(version >= INNO_VERSION_EXT(3, 0, 6, 1)) {
		is >> util::encoded_string(default_serial, version.codepage());
		if(version < INNO_VERSION(5, 2, 5)) {
			is >> util::binary_string(compiled_code);
		}
	} else {
		default_serial.clear(), compiled_code.clear();
	}
	if(version >= INNO_VERSION(4, 2, 4)) {
		is >> util::encoded_string(app_readme_file, version.codepage());
		is >> util::encoded_string(app_contact, version.codepage());
		is >> util::encoded_string(app_comments, version.codepage());
		is >> util::encoded_string(app_modify_path, version.codepage());
	} else {
		app_readme_file.clear(), app_contact.clear();
		app_comments.clear(), app_modify_path.clear();
	}
	if(version >= INNO_VERSION(5, 3, 8)) {
		is >> util::encoded_string(create_uninstall_registry_key, version.codepage());
	} else {
		create_uninstall_registry_key.clear();
	}
	if(version >= INNO_VERSION(5, 3, 10)) {
		is >> util::encoded_string(uninstallable, version.codepage());
	} else {
		uninstallable.clear();
	}
	
	if(version >= INNO_VERSION(5, 5, 6)) {
			is >> util::encoded_string(setupmutex_filter, version.codepage());
	} else {
			setupmutex_filter.clear();
	} 	
	
	if(version >= INNO_VERSION(5, 5, 0)) {
		is >> util::encoded_string(close_applications_filter, version.codepage());
	} else {
		close_applications_filter.clear();
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> util::ansi_string(license_text);
		is >> util::ansi_string(info_before);
		is >> util::ansi_string(info_after);
	}
	if(version >= INNO_VERSION(5, 2, 1) && version < INNO_VERSION(5, 3, 10)) {
		is >> util::binary_string(uninstaller_signature);
	} else {
		uninstaller_signature.clear();
	}
	if(version >= INNO_VERSION(5, 2, 5)) {
		is >> util::binary_string(compiled_code);
	}
	
	if(version >= INNO_VERSION(2, 0, 6) && !version.unicode) {
		lead_bytes = stored_char_set(is);
	} else {
		lead_bytes = 0;
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		language_count = util::load<boost::uint32_t>(is);
	} else if(version >= INNO_VERSION(2, 0, 1)) {
		language_count = 1;
	} else {
		language_count = 0;
	}
	
	if(version >= INNO_VERSION(4, 2, 1)) {
		message_count = util::load<boost::uint32_t>(is);
	} else {
		message_count = 0;
	}
	
	if(version >= INNO_VERSION(4, 1, 0)) {
		permission_count = util::load<boost::uint32_t>(is);
	} else {
		permission_count = 0;
	}
	
	if(version >= INNO_VERSION(2, 0, 0)) {
		type_count = util::load<boost::uint32_t>(is);
		component_count = util::load<boost::uint32_t>(is);
		task_count = util::load<boost::uint32_t>(is);
	} else {
		type_count = 0, component_count = 0, task_count = 0;
	}
	
	directory_count = util::load<boost::uint32_t>(is, version.bits);
	file_count = util::load<boost::uint32_t>(is, version.bits);
	data_entry_count = util::load<boost::uint32_t>(is, version.bits);
	icon_count = util::load<boost::uint32_t>(is, version.bits);
	ini_entry_count = util::load<boost::uint32_t>(is, version.bits);
	registry_entry_count = util::load<boost::uint32_t>(is, version.bits);
	delete_entry_count = util::load<boost::uint32_t>(is, version.bits);
	uninstall_delete_entry_count = util::load<boost::uint32_t>(is, version.bits);
	run_entry_count = util::load<boost::uint32_t>(is, version.bits);
	uninstall_run_entry_count = util::load<boost::uint32_t>(is, version.bits);
	
	boost::int32_t license_size = 0;
	boost::int32_t info_before_size = 0;
	boost::int32_t info_after_size = 0;
	if(version < INNO_VERSION(1, 3, 21)) {
		license_size = util::load<boost::int32_t>(is, version.bits);
		info_before_size = util::load<boost::int32_t>(is, version.bits);
		info_after_size = util::load<boost::int32_t>(is, version.bits);
	}
	
	winver.load(is, version);
	
	back_color = util::load<boost::uint32_t>(is);
	if(version >= INNO_VERSION(1, 3, 21)) {
		back_color2 = util::load<boost::uint32_t>(is);
	} else {
		back_color2 = 0;
	}
	image_back_color = util::load<boost::uint32_t>(is);
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 4)) {
		small_image_back_color = util::load<boost::uint32_t>(is);
	} else {
		small_image_back_color = 0;
	}
	
	if(version < INNO_VERSION(4, 2, 0)) {
		password.crc32 = util::load<boost::uint32_t>(is);
		password.type = crypto::CRC32;
	} else if(version < INNO_VERSION(5, 3, 9)) {
		is.read(password.md5, std::streamsize(sizeof(password.md5)));
		password.type = crypto::MD5;
	} else {
		is.read(password.sha1, std::streamsize(sizeof(password.sha1)));
		password.type = crypto::SHA1;
	}
	if(version >= INNO_VERSION(4, 2, 2)) {
		is.read(password_salt, std::streamsize(sizeof(password_salt)));
	} else {
		std::memset(password_salt, 0, sizeof(password_salt));
	}
	
	if(version >= INNO_VERSION(4, 0, 0)) {
		extra_disk_space_required = util::load<boost::int64_t>(is);
		slices_per_disk = util::load<boost::uint32_t>(is);
	} else {
		extra_disk_space_required = util::load<boost::int32_t>(is);
		slices_per_disk = 1;
	}
	
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 0)) {
		install_mode = stored_enum<stored_install_verbosity>(is).get();
	} else {
		install_mode = NormalInstallMode;
	}
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		uninstall_log_mode = stored_enum<stored_log_mode>(is).get();
	} else {
		uninstall_log_mode = AppendLog;
	}
	
	if(version >= INNO_VERSION(2, 0, 0) && version < INNO_VERSION(5, 0, 0)) {
		uninstall_style = stored_enum<stored_setup_style>(is).get();
	} else {
		uninstall_style = (version < INNO_VERSION(5, 0, 0)) ? ClassicStyle : ModernStyle;
	}
	
	if(version >= INNO_VERSION(1, 3, 21)) {
		dir_exists_warning = stored_enum<stored_bool_auto_no_yes>(is).get();
	} else {
		dir_exists_warning = Auto;
	}
	
	if(version >= INNO_VERSION(3, 0, 0) && version < INNO_VERSION(3, 0, 3)) {
		auto_bool val = stored_enum<stored_bool_auto_no_yes>(is).get();
		switch(val) {
			case Yes: options |= AlwaysRestart; break;
			case Auto: options |= RestartIfNeededByRun; break;
			case No: break;
		}
	}
	
	if(version >= INNO_VERSION(5, 3, 7)) {
		privileges_required = stored_enum<stored_privileges_1>(is).get();
	} else if(version >= INNO_VERSION(3, 0, 4)) {
		privileges_required = stored_enum<stored_privileges_0>(is).get();
	}
	
	if(version >= INNO_VERSION(4, 0, 10)) {
		show_language_dialog = stored_enum<stored_bool_yes_no_auto>(is).get();
		language_detection = stored_enum<stored_language_detection_method>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 3, 9)) {
		compression = stored_enum<stored_compression_method_3>(is).get();
	} else if(version >= INNO_VERSION(4, 2, 6)) {
		compression = stored_enum<stored_compression_method_2>(is).get();
	} else if(version >= INNO_VERSION(4, 2, 5)) {
		compression = stored_enum<stored_compression_method_1>(is).get();
	} else if(version >= INNO_VERSION(4, 1, 5)) {
		compression = stored_enum<stored_compression_method_0>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 1, 0)) {
		architectures_allowed = stored_flags<stored_architectures>(is).get();
		architectures_installed_in_64bit_mode = stored_flags<stored_architectures>(is).get();
	} else {
		architectures_allowed = architecture_types::all();
		architectures_installed_in_64bit_mode = architecture_types::all();
	}
	
	if(version >= INNO_VERSION(5, 2, 1) && version < INNO_VERSION(5, 3, 10)) {
		signed_uninstaller_original_size = util::load<boost::uint32_t>(is);
		signed_uninstaller_header_checksum = util::load<boost::uint32_t>(is);
	} else {
		signed_uninstaller_original_size = signed_uninstaller_header_checksum = 0;
	}
	
	if(version >= INNO_VERSION(5, 3, 3)) {
		disable_dir_page = stored_enum<stored_bool_auto_no_yes>(is).get();
		disable_program_group_page = stored_enum<stored_bool_auto_no_yes>(is).get();
	}
	
	if(version >= INNO_VERSION(5, 5, 0)) {
		uninstall_display_size = util::load<boost::uint64_t>(is);
	} else if(version >= INNO_VERSION(5, 3, 6)) {
		uninstall_display_size = util::load<boost::uint32_t>(is);
	} else {
		uninstall_display_size = 0;
	}
	
	if(version == INNO_VERSION_EXT(5, 5, 0, 1)) {
		/*
		 * This is needed to extract an Inno Setup variant (BlackBox v2?) that uses
		 * the 5.5.0 (unicode) data version string while the format differs:
		 * The language entries are off by one byte and the EncryptionUsed flag
		 * gets set while there is no decrypt_dll.
		 * I'm not sure where exactly this byte goes, but it's after the compression
		 * type and before EncryptionUsed flag.
		 * The other values/flags between here and there look sane (mostly default).
		 */
		(void)util::load<boost::uint8_t>(is);
	}
	
	stored_flag_reader<flags> flagreader(is, version.bits);
	
	flagreader.add(DisableStartupPrompt);
	if(version < INNO_VERSION(5, 3, 10)) {
		flagreader.add(Uninstallable);
	}
	flagreader.add(CreateAppDir);
	if(version < INNO_VERSION(5, 3, 3)) {
		flagreader.add(DisableDirPage);
	}
	if(version < INNO_VERSION(1, 3, 21)) {
		flagreader.add(DisableDirExistsWarning);
	}
	if(version < INNO_VERSION(5, 3, 3)) {
		flagreader.add(DisableProgramGroupPage);
	}
	flagreader.add(AllowNoIcons);
	if(version < INNO_VERSION(3, 0, 0) || version >= INNO_VERSION(3, 0, 3)) {
		flagreader.add(AlwaysRestart);
	}
	if(version < INNO_VERSION(1, 3, 21)) {
		flagreader.add(BackSolid);
	}
	flagreader.add(AlwaysUsePersonalGroup);
	flagreader.add(WindowVisible);
	flagreader.add(WindowShowCaption);
	flagreader.add(WindowResizable);
	flagreader.add(WindowStartMaximized);
	flagreader.add(EnableDirDoesntExistWarning);
	if(version < INNO_VERSION(4, 1, 2)) {
		flagreader.add(DisableAppendDir);
	}
	flagreader.add(Password);
	flagreader.add(AllowRootDirectory);
	flagreader.add(DisableFinishedPage);
	if(version.bits != 16) {
		if(version < INNO_VERSION(3, 0, 4)) {
			flagreader.add(AdminPrivilegesRequired);
		}
		if(version < INNO_VERSION(3, 0, 0)) {
			flagreader.add(AlwaysCreateUninstallIcon);
		}
		if(version < INNO_VERSION(1, 3, 21)) {
			flagreader.add(OverwriteUninstRegEntries);
		}
		flagreader.add(ChangesAssociations);
	}
	if(version >= INNO_VERSION(1, 3, 21)) {
		if(version < INNO_VERSION(5, 3, 8)) {
			flagreader.add(CreateUninstallRegKey);
		}
		flagreader.add(UsePreviousAppDir);
		flagreader.add(BackColorHorizontal);
		flagreader.add(UsePreviousGroup);
		flagreader.add(UpdateUninstallLogAppName);
	}
	if(version >= INNO_VERSION(2, 0, 0)) {
		flagreader.add(UsePreviousSetupType);
		flagreader.add(DisableReadyMemo);
		flagreader.add(AlwaysShowComponentsList);
		flagreader.add(FlatComponentsList);
		flagreader.add(ShowComponentSizes);
		flagreader.add(UsePreviousTasks);
		flagreader.add(DisableReadyPage);
	}
	if(version >= INNO_VERSION(2, 0, 7)) {
		flagreader.add(AlwaysShowDirOnReadyPage);
		flagreader.add(AlwaysShowGroupOnReadyPage);
	}
	if(version >= INNO_VERSION(2, 0, 17) && version < INNO_VERSION(4, 1, 5)) {
		flagreader.add(BzipUsed);
	}
	if(version >= INNO_VERSION(2, 0, 18)) {
		flagreader.add(AllowUNCPath);
	}
	if(version >= INNO_VERSION(3, 0, 0)) {
		flagreader.add(UserInfoPage);
		flagreader.add(UsePreviousUserInfo);
	}
	if(version >= INNO_VERSION(3, 0, 1)) {
		flagreader.add(UninstallRestartComputer);
	}
	if(version >= INNO_VERSION(3, 0, 3)) {
		flagreader.add(RestartIfNeededByRun);
	}
	if(version >= INNO_VERSION_EXT(3, 0, 6, 1)) {
		flagreader.add(ShowTasksTreeLines);
	}
	if(version >= INNO_VERSION(4, 0, 0) && version < INNO_VERSION(4, 0, 10)) {
		flagreader.add(ShowLanguageDialog);
	}
	if(version >= INNO_VERSION(4, 0, 1) && version < INNO_VERSION(4, 0, 10)) {
		flagreader.add(DetectLanguageUsingLocale);
	}
	if(version >= INNO_VERSION(4, 0, 9)) {
		flagreader.add(AllowCancelDuringInstall);
	} else {
		options |= AllowCancelDuringInstall;
	}
	if(version >= INNO_VERSION(4, 1, 3)) {
		flagreader.add(WizardImageStretch);
	}
	if(version >= INNO_VERSION(4, 1, 8)) {
		flagreader.add(AppendDefaultDirName);
		flagreader.add(AppendDefaultGroupName);
	}
	if(version >= INNO_VERSION(4, 2, 2)) {
		flagreader.add(EncryptionUsed);
	}
	if(version >= INNO_VERSION(5, 0, 4)) {
		flagreader.add(ChangesEnvironment);
	}
	if(version >= INNO_VERSION(5, 1, 7) && !version.unicode) {
		flagreader.add(ShowUndisplayableLanguages);
	}
	if(version >= INNO_VERSION(5, 1, 13)) {
		flagreader.add(SetupLogging);
	}
	if(version >= INNO_VERSION(5, 2, 1)) {
		flagreader.add(SignedUninstaller);
	}
	if(version >= INNO_VERSION(5, 3, 8)) {
		flagreader.add(UsePreviousLanguage);
	}
	if(version >= INNO_VERSION(5, 3, 9)) {
		flagreader.add(DisableWelcomePage);
	}
	if(version >= INNO_VERSION(5, 5, 0)) {
		flagreader.add(CloseApplications);
		flagreader.add(RestartApplications);
		flagreader.add(AllowNetworkDrive);
	} else {
		options |= AllowNetworkDrive;
	}
	
	options |= flagreader;
	
	if(version < INNO_VERSION(3, 0, 4)) {
		privileges_required = (options & AdminPrivilegesRequired) ? AdminPriviliges : NoPrivileges;
	}
	
	if(version < INNO_VERSION(4, 0, 10)) {
		show_language_dialog = (options & ShowLanguageDialog) ? Yes : No;
		language_detection = (options & DetectLanguageUsingLocale) ? LocaleLanguage : UILanguage;
	}
	
	if(version < INNO_VERSION(4, 1, 5)) {
		compression = (options & BzipUsed) ? stream::BZip2 : stream::Zlib;
	}
	
	if(version < INNO_VERSION(5, 3, 3)) {
		disable_dir_page = (options & DisableDirPage) ? Yes : No;
		disable_program_group_page = (options & DisableProgramGroupPage) ? Yes : No;
	}
	
	if(version < INNO_VERSION(1, 3, 21)) {
		if(license_size > 0) {
			std::string temp;
			temp.resize(size_t(license_size));
			is.read(&temp[0], license_size);
			util::to_utf8(temp, license_text);
		}
		if(info_before_size > 0) {
			std::string temp;
			temp.resize(size_t(info_before_size));
			is.read(&temp[0], info_before_size);
			util::to_utf8(temp, info_before);
		}
		if(info_after_size > 0) {
			std::string temp;
			temp.resize(size_t(info_after_size));
			is.read(&temp[0], info_after_size);
			util::to_utf8(temp, info_after);
		}
	}
	
}

} // namespace setup

NAMES(setup::header::flags, "Setup Option",
	"disable startup prompt",
	"create app dir",
	"allow no icons",
	"always restart",
	"always use personal group",
	"window visible",
	"window show caption",
	"window resizable",
	"window start maximized",
	"enable dir doesn't exist warning",
	"password",
	"allow root directory",
	"disable finished page",
	"changes associations",
	"use previous app dir",
	"back color horizontal",
	"use previous group",
	"update uninstall log app name",
	"use previous setup type",
	"disable ready memo",
	"always show components list",
	"flat components list",
	"show component sizes",
	"use previous tasks",
	"disable ready page",
	"always show dir on ready page",
	"always show group on ready page",
	"allow unc path",
	"user info page",
	"use previous user info",
	"uninstall restart computer",
	"restart if needed by run",
	"show tasks tree lines",
	"allow cancel during install",
	"wizard image stretch",
	"append default dir name",
	"append default group name",
	"encrypted",
	"changes environment",
	"show undisplayable languages",
	"setup logging",
	"signed uninstaller",
	"use previous language",
	"disable welcome page",
	"close applications",
	"restart applications",
	"allow network drive",
	"uninstallable",
	"disable dir page",
	"disable program group page",
	"disable append dir",
	"admin privilegesrequired",
	"always create uninstall icon",
	"create uninstall reg key",
	"bzip used",
	"show language dialog",
	"detect language using locale",
	"disable dir exists warning",
	"back solid",
	"overwrite uninst reg entries",
	"unknown flags",
)

NAMES(setup::header::architecture_types, "Architecture",
	"unknown",
	"x86",
	"amd64",
	"IA64",
)

NAMES(setup::header::install_verbosity, "Install Mode",
	"normal",
	"silent",
	"very silent",
)

NAMES(setup::header::log_mode, "Uninstall Log Mode",
	"append",
	"new log",
	"overwrite",
)

NAMES(setup::header::style, "Uninstall Style",
	"classic",
	"modern",
)

NAMES(setup::header::auto_bool, "Auto Boolean",
	"auto",
	"no",
	"yes",
)

NAMES(setup::header::privilege_level, "Privileges",
	"none",
	"power user",
	"admin",
	"lowest",
)

NAMES(setup::header::language_detection_method, "Language Detection",
	"ui language",
	"locale",
	"none",
)
