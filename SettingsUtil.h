#pragma once

void SaveSettings()
{
	using rapidjson::UTF8;
	using rapidjson::UTF16;

	rapidjson::StringBuffer json;
	rapidjson::Writer<rapidjson::StringBuffer> writer(json);

	writer.StartObject();

	writer.Key("language");
	writer.Uint(g_browserSettings.language);

	writer.Key("codePage");
	writer.Uint(g_browserSettings.codePage);

	writer.Key("playerName");
	writer.String(g_browserSettings.playerName);

	writer.Key("gamePath");
	rapidjson::GenericStringStream<UTF16<>> source(g_browserSettings.gamePath.c_str());
	rapidjson::StringBuffer target;
	bool hasError = false;
	while (source.Peek() != '\0') if (!rapidjson::Transcoder<UTF16<>, UTF8<>>::Transcode(source, target)) { hasError = true; break; }
	writer.String(hasError ? "" : target.GetString());

	writer.Key("gameUpdateFreq");
	writer.Uint(g_browserSettings.gameUpdateFreq);

	writer.Key("gameUpdateURL");
	writer.String(g_browserSettings.gameUpdateURL);

	writer.Key("gameUpdatePassword");
	writer.String(g_browserSettings.gameUpdatePassword);

	writer.Key("masterlistURL");
	writer.String(g_browserSettings.masterlistURL);

	writer.Key("proxy");
	writer.String(g_browserSettings.proxy);

	writer.Key("officialColor");
	writer.Uint(g_browserSettings.officialColor);

	writer.Key("custColors");
	writer.StartArray();
	for (size_t i = 0; i < 16; ++i)
		writer.Uint(g_browserSettings.custColors[i]);
	writer.EndArray();

	writer.EndObject();

	SetCurrentDirectory(g_exePath);

	FILE *file = _wfopen(L"settings.json", L"wb");
	if (file != nullptr)
	{
		fwrite(json.GetString(), sizeof(char), json.GetSize(), file);
		fclose(file);
	}
}

void DefaultSettings()
{
	memset(g_browserSettings.playerName, 0, sizeof(g_browserSettings.playerName));
	g_browserSettings.gamePath = L"";
	g_browserSettings.gameUpdateFreq = START;
	g_browserSettings.gameUpdateURL = "http://u04.maxorator.com";
	g_browserSettings.gameUpdatePassword = "";
	g_browserSettings.masterlistURL = "http://master.vc-mp.ovh";
	g_browserSettings.proxy = "";
	g_browserSettings.officialColor = RGB(0, 0, 255); // Blue
	std::fill_n(g_browserSettings.custColors, std::size(g_browserSettings.custColors), 0xFFFFFF); // White
}

void LoadSettings()
{
	DefaultSettings();

	SetCurrentDirectory(g_exePath);

	FILE *file = _wfopen(L"settings.json", L"rb");
	if (file != nullptr)
	{
		do {
			char readBuffer[512];
			rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

			rapidjson::Document dom;

			if (dom.ParseStream(is).HasParseError() || !dom.IsObject())
				break;

			auto member = dom.FindMember("language");
			if (member != dom.MemberEnd() && member->value.IsUint())
				g_browserSettings.language = member->value.GetUint();

			member = dom.FindMember("codePage");
			if (member != dom.MemberEnd() && member->value.IsUint())
				g_browserSettings.codePage = member->value.GetUint();

			member = dom.FindMember("playerName");
			if (member != dom.MemberEnd() && member->value.IsString())
				strncpy(g_browserSettings.playerName, member->value.GetString(), sizeof(g_browserSettings.playerName));

			member = dom.FindMember("gamePath");
			if (member != dom.MemberEnd() && member->value.IsString())
			{
				using rapidjson::UTF8;
				using rapidjson::UTF16;

				rapidjson::GenericStringStream<UTF8<>> source(member->value.GetString());
				rapidjson::GenericStringBuffer<UTF16<>> target;
				bool hasError = false;
				while (source.Peek() != '\0') if (!rapidjson::Transcoder<UTF8<>, UTF16<>>::Transcode(source, target)) { hasError = true; break; }
				g_browserSettings.gamePath = std::wstring(target.GetString(), target.GetSize());
			}

			member = dom.FindMember("gameUpdateFreq");
			if (member != dom.MemberEnd() && member->value.IsUint())
				g_browserSettings.gameUpdateFreq = (updateFreq)member->value.GetUint();

			member = dom.FindMember("gameUpdateURL");
			if (member != dom.MemberEnd() && member->value.IsString())
				g_browserSettings.gameUpdateURL = std::string(member->value.GetString(), member->value.GetStringLength());

			member = dom.FindMember("gameUpdatePassword");
			if (member != dom.MemberEnd() && member->value.IsString())
				g_browserSettings.gameUpdatePassword = std::string(member->value.GetString(), member->value.GetStringLength());

			member = dom.FindMember("masterlistURL");
			if (member != dom.MemberEnd() && member->value.IsString())
				g_browserSettings.masterlistURL = std::string(member->value.GetString(), member->value.GetStringLength());

			member = dom.FindMember("proxy");
			if (member != dom.MemberEnd() && member->value.IsString())
				g_browserSettings.proxy = std::string(member->value.GetString(), member->value.GetStringLength());

			member = dom.FindMember("officialColor");
			if (member != dom.MemberEnd() && member->value.IsUint())
				g_browserSettings.officialColor = member->value.GetUint();

			member = dom.FindMember("custColors");
			if (member != dom.MemberEnd() && member->value.IsArray())
			{
				size_t i = 0;
				for (auto it = member->value.Begin(); it != member->value.End() && i < 16; ++it, ++i)
				{
					if (it->IsUint())
						g_browserSettings.custColors[i] = it->GetUint();
				}
			}
		} while (0);
		fclose(file);
	}
}

void SaveHistory()
{
	rapidjson::StringBuffer json;
	rapidjson::Writer<rapidjson::StringBuffer> writer(json);

	writer.StartArray();

	for (serverAllInfo server : g_historyList)
	{
		writer.StartObject();

		char ipstr[16];
		char *ip = (char *)&(server.address.ip);
		snprintf(ipstr, sizeof(ipstr), "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);

		writer.Key("ip");
		writer.String(ipstr);

		writer.Key("port");
		writer.Uint(server.address.port);

		writer.Key("serverName");
		writer.String(server.info.serverName);

		writer.Key("isOfficial");
		writer.Bool(server.isOfficial);

		writer.Key("lastPlayed");
		writer.Uint64(server.lastPlayed);

		writer.EndObject();
	}

	writer.EndArray();

	SetCurrentDirectory(g_exePath);

	FILE *file = _wfopen(L"history.json", L"wb");
	if (file != nullptr)
	{
		fwrite(json.GetString(), sizeof(char), json.GetSize(), file);
		fclose(file);
	}
}

void LoadHistory()
{
	FILE *file = _wfopen(L"history.json", L"rb");
	if (file != nullptr)
	{
		do {
			char readBuffer[512];
			rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

			rapidjson::Document dom;

			if (dom.ParseStream(is).HasParseError() || !dom.IsArray())
				break;

			for (auto it = dom.Begin(); it != dom.End(); ++it)
			{
				if (it->IsObject())
				{
					serverAllInfo server = {};
					auto ip = it->FindMember("ip");
					auto port = it->FindMember("port");
					auto serverName = it->FindMember("serverName");
					auto isOfficial = it->FindMember("isOfficial");
					auto lastPlayed = it->FindMember("lastPlayed");
					if (ip != it->MemberEnd() && port != it->MemberEnd() && serverName != it->MemberEnd() && isOfficial != it->MemberEnd() && lastPlayed != it->MemberEnd() && ip->value.IsString() && port->value.IsUint() && serverName->value.IsString() && isOfficial->value.IsBool() && lastPlayed->value.IsUint())
					{
						server.address.ip = inet_addr(ip->value.GetString());
						server.address.port = (uint16_t)port->value.GetUint();
						server.info.serverName = serverName->value.GetString();
						server.isOfficial = isOfficial->value.GetBool();
						server.lastPlayed = lastPlayed->value.GetUint64();
						server.lastRecv = 0;
						g_historyList.push_back(server);
					}
				}
				else
					break;
			}
		} while (0);
		fclose(file);
	}
}

void SaveFavorites()
{
	rapidjson::StringBuffer json;
	rapidjson::Writer<rapidjson::StringBuffer> writer(json);

	writer.StartArray();

	for (serverAllInfo server : g_favoriteList)
	{
		writer.StartObject();

		char ipstr[16];
		char *ip = (char *)&(server.address.ip);
		snprintf(ipstr, sizeof(ipstr), "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);

		writer.Key("ip");
		writer.String(ipstr);

		writer.Key("port");
		writer.Uint(server.address.port);

		writer.Key("serverName");
		writer.String(server.info.serverName);

		writer.Key("isOfficial");
		writer.Bool(server.isOfficial);

		writer.EndObject();
	}

	writer.EndArray();

	SetCurrentDirectory(g_exePath);

	FILE *file = _wfopen(L"favorites.json", L"wb");
	if (file != nullptr)
	{
		fwrite(json.GetString(), sizeof(char), json.GetSize(), file);
		fclose(file);
	}
}

void LoadFavorites()
{
	FILE *file = _wfopen(L"favorites.json", L"rb");
	if (file != nullptr)
	{
		do {
			char readBuffer[512];
			rapidjson::FileReadStream is(file, readBuffer, sizeof(readBuffer));

			rapidjson::Document dom;

			if (dom.ParseStream(is).HasParseError() || !dom.IsArray())
				break;

			for (auto it = dom.Begin(); it != dom.End(); ++it)
			{
				if (it->IsObject())
				{
					serverAllInfo server = {};
					auto ip = it->FindMember("ip");
					auto port = it->FindMember("port");
					auto serverName = it->FindMember("serverName");
					auto isOfficial = it->FindMember("isOfficial");
					auto lastPlayed = it->FindMember("lastPlayed");
					if (ip != it->MemberEnd() && port != it->MemberEnd() && serverName != it->MemberEnd() && isOfficial != it->MemberEnd() && ip->value.IsString() && port->value.IsUint() && serverName->value.IsString() && isOfficial->value.IsBool())
					{
						server.address.ip = inet_addr(ip->value.GetString());
						server.address.port = (uint16_t)port->value.GetUint();
						server.info.serverName = serverName->value.GetString();
						server.isOfficial = isOfficial->value.GetBool();
						server.lastRecv = 0;
						g_favoriteList.push_back(server);
					}
				}
				else
					break;
			}
		} while (0);
		fclose(file);
	}
}

bool ImportFavorite(const wchar_t *filename, std::vector<serverHost> &serverHosts)
{
	using std::ifstream;
	ifstream file(filename, ifstream::binary);
	if (file)
	{
		file.exceptions(ifstream::failbit | ifstream::badbit);
		try
		{
			char magic[8];
			file.read(magic, sizeof(magic));
			if (*(uint64_t*)&magic == 0x10100544655FFFF) // FF FF 55 46 54 00 01 01
			{
				uint32_t count;
				file.read((char*)&count, sizeof(count));
				for (uint32_t i = 0; i < count; i++)
				{
					uint16_t len;
					file.read((char*)&len, sizeof(len));

					char * hostname = new char[len];
					file.read(hostname, len);

					uint16_t port;
					file.read((char*)&port, sizeof(port));

					serverHost host = { hostname, port };
					serverHosts.push_back(host);

					delete[] hostname;
				}
				return true;
			}
		}
		catch (ifstream::failure e)
		{
			return false;
		}
	}
	return false;
}
