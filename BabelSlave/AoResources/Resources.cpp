#include <iostream>
#include <fstream>
#include <string>
#include "Utils//FileUtils.h"
#include <vector>
#include "Utils/IniReader.h"
#include <thread>
#include "Resources.hpp"
#include "Compresor.hpp"

using namespace std;
namespace AO
{
    enum E_Heading
    {
        E_Heading_NORTH = 0,
        E_Heading_EAST = 1,
        E_Heading_SOUTH = 2,
        E_Heading_WEST = 3
    };
    namespace {

        const char* HeadDataPath = "init/cabezas.ind";
        const char* GrhIniPath = "init/graficos.ini";
        const char* BodyDataPath = "init/cuerpos.dat";
        const char* LocalIndexPath = "init/localindex.dat";
        const char* BodyStructPath = "init/moldes.ini";
        const char* HelmDataPath = "init/cascos.ind";
        const char* WeaponDataPath = "init/armas.dat";
        const char* ShieldDataPath = "init/escudos.dat";

        const int TilePixelHeight = 32;
        const int TilePixelWidth = 32;
    }
#pragma pack(push, 1)

    struct GrhData {
        int16_t sX;
        int16_t sY;
        int16_t FileNum;
        int16_t pixelWidth;
        int16_t pixelHeight;
        float TileWidth;
        float TileHeight;
        int16_t NumFrames;
        std::vector<int> Frames;
        int16_t speed;
        int32_t mini_map_color;
    };

    struct Grh {
        int GrhIndex;
        float framecounter;
        float speed;
        int Started;
        uint16_t alpha_blend;
        float Angle;
        int Loops;
    };

    struct Header {
        char desc[255];
        int CRC;
        int MagicWord;
    };

    struct HeadIndex {
        int Head[4];
    };

    struct HeadData {
        Grh Head[4];
    };

    struct WeaponAnimData {
        Grh WeaponWalk[4];
    };

    struct ShieldAnimData {
        Grh Heading[4];
    };

    struct MoldeCuerpo {
        int32_t x;
        int32_t y;
        int32_t Width;
        int32_t Height;
        uint8_t DirCount[4];
        int32_t TotalGrhs;
    };
    struct Position {
        int Y;
        int X;
    };

    struct BodyData {
        Grh Walk[4];
        Position HeadOffset;
        Position BodyOffset;
    };

#pragma pack(pop) // restore previous packing setting

    class ResourceLoader {
    public:
        ~ResourceLoader() { if (mLoadingThread.joinable()) mLoadingThread.join(); }
        void StartLoading(bool compressed);

        void GetBodyInfo(CharacterRenderInfo& charInfo, int bodyIndex, int headIndex, int helmIndex, int shieldIndex, int weaponIndex);
        void GetHeadInfo(GrhDetails& headInfo, int headIndex);
        void SetGrhDetails(GrhDetails& dest, int grhIndex);
        void GetSpellData(SpellData& dest, int spellIndex);
        void GetObjectData(ObjectData& dest, int objIndex);
        void GetNpcinfo(NpcInfo& dest, int npcIndex);
    private:
        void InitGrh(Grh& grhObj, int GrhIndex, int Started = -1, int16_t Loops = -1);
        void InitGrhWithBody(GrhData& dest, const MoldeCuerpo& bodyData, int16_t fileNum, int16_t x, int16_t y, int index);
        void LoadGrhIni();
        void LoadHeads();
        void LoadBodyStruct();
        void LoadHelm();
        void LoadBodies();
        void LoadWeaponAnimations();
        void LoadShields();
        void LoadLocalIndex();
        void LoadNpcData(INIReader& reader);
        void LoadSpellData(INIReader& reader);
        void LoadItemData(INIReader& reader);
        bool GetFile(const char* fileName, std::vector<uint8_t>& fileData);
        void UnloadCompressedFiles();
    private:
        std::vector<GrhData> mGrhData;
        std::vector<MoldeCuerpo> mBodyStructData;
        std::vector<HeadIndex> mHeadList;
        std::vector<HeadData> mHeadData;
        std::vector<HeadData> mHelmAnimData;
        std::vector<BodyData> mBodyData;
        std::vector<WeaponAnimData> mWeaponAnimData;
        std::vector<ShieldAnimData> mShieldAnimData;
        std::vector<SpellData> mSpellInfo;
        std::vector<NpcInfo> mNpcInfo;
        std::vector<ObjectData> mObjData;
        std::map<std::string, std::unique_ptr<Compressor>> mCompressedFiles;
        E_Heading mBodyHeading[4];
        bool mCompressed;

        std::thread mLoadingThread;
    };


    void ResourceLoader::InitGrh(Grh& grhObj, int GrhIndex, int Started, int16_t Loops)
    {
        if (GrhIndex < 0 || GrhIndex >= mGrhData.size())
        {
            grhObj.GrhIndex = -1;
            return;
        }

        grhObj.GrhIndex = GrhIndex;

        if (mGrhData[GrhIndex].NumFrames > 1)
        {
            grhObj.Started = (Started >= 0) ? Started : 0;
            grhObj.Loops = Loops;
            grhObj.speed = mGrhData[GrhIndex].speed / mGrhData[GrhIndex].NumFrames;
        }
        else
        {
            grhObj.Started = 0;
            grhObj.speed = 1;
        }
    }

    void ResourceLoader::InitGrhWithBody(GrhData& dest, const MoldeCuerpo& bodyData, int16_t fileNum, int16_t x, int16_t y, int index)
    {
        dest.FileNum = fileNum;
        dest.NumFrames = 1;
        dest.sX = x;
        dest.sY = y;
        dest.pixelWidth = bodyData.Width;
        dest.pixelHeight = bodyData.Height;
        dest.TileWidth = dest.pixelWidth / TilePixelWidth;
        dest.TileHeight = dest.pixelHeight / TilePixelHeight;
        dest.Frames.resize(1);
        dest.Frames[0] = index;
    }


    std::vector<std::string> splitString(const std::string& inputString, char delimiter) {
        std::vector<std::string> result;
        std::string token;
        for (const char& c : inputString) {
            if (c == delimiter) {
                result.push_back(token);
                token.clear();
            }
            else {
                token += c;
            }
        }
        result.push_back(token);
        return result;
    }

    std::string to_uppercase(std::string str) {
        std::locale loc;
        for (auto& c : str) {
            c = std::toupper(c, loc);
        }
        return str;
    }

    void ResourceLoader::LoadGrhIni()
    {
        long grh = 0, Frame = 0;
        char SeparadorClave = '=';
        char SeparadorGrh = '-';
        std::vector<uint8_t> fileData;
        // Abrimos el archivo. No uso FileManager porque obliga a cargar todo el archivo en memoria
        // y es demasiado grande. En cambio leo linea por linea y procesamos de a una.
        if (!GetFile(GrhIniPath, fileData))
        {
            return;
        }
        char* lineStart = reinterpret_cast<char*>(fileData.data());
        char* lineEnd;
        char* fileEnd = lineStart + fileData.size();
        // Leemos el total de Grhs
        while (lineStart < fileEnd)
        {
            lineEnd = GetLineEnd(lineStart);
            char* value = strchr(lineStart, SeparadorClave);
            *lineEnd = 0;

            // Buscamos la clave "NumGrh"
            if ( std::string(lineStart, value - lineStart) == "NumGrh")
            {
                lineEnd = GetLineEnd(lineStart);
                value++;
                // Asignamos el tamano al array de Grhs
                auto MaxGrh = stoi(value);

                mGrhData.resize(MaxGrh);
                lineStart = GetNextLine(lineEnd, fileEnd);
                break;
            }
            lineStart = GetNextLine(lineEnd, fileEnd);
        }
        // Chequeamos si pudimos leer la cantidad de Grhs
        if (mGrhData.size() <= 0) return;
        // Buscamos la posicion del primer Grh
        while (lineStart < fileEnd)
        {
            lineEnd = GetLineEnd(lineStart);
            *lineEnd = 0;
            if (to_uppercase(lineStart) == "[GRAPHICS]") // Buscamos el nodo "[Graphics]"
            {
                lineStart = GetNextLine(lineEnd, fileEnd);
                break;
            }
            lineStart = GetNextLine(lineEnd, fileEnd);
        }

        // Recorremos todos los Grhs
        while (lineStart < fileEnd)
        {
            lineEnd = GetLineEnd(lineStart);
            *lineEnd = 0;
            Frame = 0;
            char* value = strchr(lineStart, SeparadorClave);
            // there is no value separator, skip the line
            if (value == nullptr) continue;
            *value = 0;
            value++;
            //skip grh part
            lineStart += 3;
            // Leemos el numero de Grh (el numero a la derecha de la palabra "Grh")
            grh = stol(lineStart) - 1;
            // Leemos los campos de datos del Grh
            char* valueEnd = strchr(value, SeparadorGrh);
            *valueEnd = 0;
            auto& gd = mGrhData[grh];
            // Primer lugar: cantidad de frames.
            gd.NumFrames = stoi(value);
            gd.Frames.resize(gd.NumFrames);
            // Tiene mas de un frame entonces es una animacion
            if (gd.NumFrames > 1)
            {
                // Segundo lugar: Leemos los numeros de grh de la animacion
                for (Frame = 1; Frame <= gd.NumFrames; Frame++)
                {
                    value = ++valueEnd;
                    valueEnd = strchr(value, SeparadorGrh);
                    *valueEnd = 0;
                    gd.Frames[Frame - 1] = stoi(value) - 1;
                    if (gd.Frames[Frame - 1] <= 0 || gd.Frames[Frame - 1] > mGrhData.size()) return;
                }
                value = ++valueEnd;
                // Tercer lugar: leemos la velocidad de la animacion
                gd.speed = stoi(value);
                if (gd.speed <= 0) return;

                // Por ultimo, copiamos las dimensiones del primer frame
                gd.pixelHeight = mGrhData[gd.Frames[0]].pixelHeight;
                gd.pixelWidth = mGrhData[gd.Frames[0]].pixelWidth;
                gd.TileWidth = mGrhData[gd.Frames[0]].pixelHeight;
            }
            else if (gd.NumFrames == 1)
            {
                // Si es un solo frame lo asignamos a si mismo
                gd.Frames[0] = grh;
                value = ++valueEnd;
                valueEnd = strchr(value, SeparadorGrh);
                *valueEnd = 0;
                // Segundo lugar: NumeroDelGrafico.bmp, pero sin el ".bmp"
                gd.FileNum = stoi(value);
                value = ++valueEnd;
                valueEnd = strchr(value, SeparadorGrh);
                *valueEnd = 0;
                // Tercer Lugar: La coordenada X del grafico
                gd.sX = stoi(value);
                value = ++valueEnd;
                valueEnd = strchr(value, SeparadorGrh);
                *valueEnd = 0;
                // Cuarto Lugar: La coordenada Y del grafico
                gd.sY = stoi(value);
                value = ++valueEnd;
                valueEnd = strchr(value, SeparadorGrh);
                *valueEnd = 0;
                // Quinto lugar: El ancho del grafico
                gd.pixelWidth = stoi(value);
                value = ++valueEnd;
                // Sexto lugar: La altura del grafico
                gd.pixelHeight = std::stoi(value);

                //Calculamos el ancho y alto en tiles
                gd.TileWidth = gd.pixelWidth / TilePixelWidth;
                gd.TileHeight = gd.pixelHeight / TilePixelHeight;
            }
            lineStart = GetNextLine(lineEnd, fileEnd);
        }
    }

    void ResourceLoader::LoadHeads() {
        int16_t n;
        int i;
        int16_t Numheads;
        n = _fileno(stdout);
        Header MiCabecera;
        // cabecera
        char* buf = reinterpret_cast<char*>(&MiCabecera);
        int currentPos = 0;
        std::vector<uint8_t> fileData;
        if (!GetFile(HeadDataPath, fileData))
        {
            return;
        }
        memcpy_s(buf, sizeof(MiCabecera), fileData.data(), sizeof(MiCabecera));
        currentPos += sizeof(MiCabecera);
        memcpy_s(&Numheads, sizeof(int16_t), fileData.data() + currentPos, sizeof(int16_t));
        currentPos += sizeof(int16_t);
        // Resize array
        mHeadData.resize(Numheads);
        mHeadList.resize(Numheads);
        memcpy_s(mHeadList.data(), sizeof(HeadIndex) * Numheads, fileData.data() + currentPos, fileData.size() - currentPos);

        for (i = 0; i < Numheads; i++) {
            for (int j = 0; j < 4; j++) mHeadList[i].Head[j] = mHeadList[i].Head[j] - 1;
            if (mHeadList[i].Head[0] >= 0)
            {
                InitGrh(mHeadData[i].Head[E_Heading_NORTH], mHeadList[i].Head[E_Heading_NORTH], 0);
                InitGrh(mHeadData[i].Head[E_Heading_EAST], mHeadList[i].Head[E_Heading_EAST] , 0);
                InitGrh(mHeadData[i].Head[E_Heading_SOUTH], mHeadList[i].Head[E_Heading_SOUTH], 0);
                InitGrh(mHeadData[i].Head[E_Heading_WEST], mHeadList[i].Head[E_Heading_WEST], 0);
            }
        }
    }

    void ResourceLoader::LoadNpcData(INIReader& reader)
    {
        int npcCount = reader.GetInteger("INIT", "NUMNPCS", 0); //NumHechizos = Val(Leer.GetValue("INIT", "NumeroHechizo"))
        mNpcInfo.resize(npcCount);
        for (int i = 0; i < npcCount; i++)
        {
            std::string spellEntry = "Npc" + std::to_string(i + 1);
            mNpcInfo[i].Name = reader.Get(spellEntry, "NAME", "");
        }
    }

    void ResourceLoader::LoadSpellData(INIReader& reader)
    {
        int spellCount = reader.GetInteger("INIT", "NUMEROHECHIZO", 0); //NumHechizos = Val(Leer.GetValue("INIT", "NumeroHechizo"))
        mSpellInfo.resize(spellCount);
        for (int i = 0; i < spellCount; i++)
        {
            std::string spellEntry = "HECHIZO" + std::to_string(i + 1);
            mSpellInfo[i].Name = reader.Get(spellEntry, "NOMBRE", "");
            mSpellInfo[i].Description = reader.Get(spellEntry, "DESC", "");
            mSpellInfo[i].RequiredMana = reader.GetInteger(spellEntry, "MANAREQUERIDO", 0);
            mSpellInfo[i].RequiredSkill = reader.GetInteger(spellEntry, "MINSKILL", 0);
            mSpellInfo[i].RequiredStamina = reader.GetInteger(spellEntry, "STAREQUERIDO", 0);
            mSpellInfo[i].IconIndex = reader.GetInteger(spellEntry, "ICONOINDEX", 0);
            mSpellInfo[i].Cooldown = reader.GetInteger(spellEntry, "COOLDOWN", 0);
        }
    }

    void ResourceLoader::LoadItemData(INIReader& reader)
    {
        int itemCount = reader.GetInteger("INIT", "NUMOBJS", 0); //NumHechizos = Val(Leer.GetValue("INIT", "NumeroHechizo"))
        mObjData.resize(itemCount);
        for (int i = 0; i < itemCount; i++)
        {
            std::string object = "OBJ" + std::to_string(i + 1);
            mObjData[i].Name = reader.Get(object, "NAME", "");
            mObjData[i].Description = reader.Get(object, "TEXTO", "");
            mObjData[i].grhindex = reader.GetInteger(object, "GRHINDEX", 0);
            mObjData[i].MinDef = reader.GetInteger(object, "MINDEF", 0);
            mObjData[i].MaxDef = reader.GetInteger(object, "MAXDEF", 0);
            mObjData[i].MinHit = reader.GetInteger(object, "MINHIT", 0);
            mObjData[i].MaxHit = reader.GetInteger(object, "MAXHIT", 0);
            mObjData[i].ObjType = reader.GetInteger(object, "OBJTYPE", 0);
            mObjData[i].CreatesLight = reader.Get(object, "CREALUZ", "");
            mObjData[i].CreateFloorParticle = reader.GetInteger(object, "CREAPARTICULAPISO", 0);
            mObjData[i].CreatesGRH = reader.Get(object, "CREAGRH", "");
            mObjData[i].Roots = reader.GetInteger(object, "RAICES", 0);
            mObjData[i].Wood = reader.GetInteger(object, "MADERA", 0);
            mObjData[i].ElvenWood = reader.GetInteger(object, "MADERAELFICA", 0);
            mObjData[i].WolfSkin = reader.GetInteger(object, "PIELLOBO", 0);
            mObjData[i].BrownBearSkin = reader.GetInteger(object, "PIELOSOPARDO", 0);
            mObjData[i].PolarBearSkin = reader.GetInteger(object, "PIELOSOPOLAR", 0);
            mObjData[i].LingH = reader.GetInteger(object, "LINGH", 0);
            mObjData[i].LingP = reader.GetInteger(object, "LINGP", 0);
            mObjData[i].LingO = reader.GetInteger(object, "LINGO", 0);
            mObjData[i].Destroy = reader.GetInteger(object, "DESTRUYE", 0);
            mObjData[i].Projectile = reader.GetInteger(object, "PROYECTIL", 0);
            mObjData[i].Ammunition = reader.GetInteger(object, "MUNICIONES", 0);
            mObjData[i].BlacksmithingSkill = reader.GetInteger(object, "SKHERRERIA", 0);
            mObjData[i].PotionSkill = reader.GetInteger(object, "SKPOCIONES", 0);
            mObjData[i].TailoringSkill = reader.GetInteger(object, "SKSASTRERIA", 0);
            mObjData[i].Value = reader.GetInteger(object, "VALOR", 0);
            mObjData[i].Grabbable = reader.GetInteger(object, "AGARRABLE", 0);
            mObjData[i].Key = reader.GetInteger(object, "LLAVE", 0);
            mObjData[i].Cooldown = reader.GetInteger(object, "CD", 0);
            mObjData[i].CdType = reader.GetInteger(object, "CDTYPE", 0);
            mObjData[i].SpellIndex = reader.GetInteger(object, "SPELLINDEX", 0);
        }
    }

    void ResourceLoader::LoadLocalIndex()
    {
        std::vector<uint8_t> fileData;
        if (!GetFile(LocalIndexPath, fileData))
        {
            return;
        }
        INIReader Loader(reinterpret_cast<char*>(fileData.data()), fileData.size());
        auto error = Loader.ParseError();
        LoadSpellData(Loader);
        LoadItemData(Loader);
        LoadNpcData(Loader);
    }

    void ResourceLoader::LoadBodyStruct()
    {
        mBodyHeading[0] = E_Heading::E_Heading_SOUTH;
        mBodyHeading[1] = E_Heading_NORTH;
        mBodyHeading[2] = E_Heading::E_Heading_WEST;
        mBodyHeading[3] = E_Heading::E_Heading_EAST;
        std::vector<uint8_t> fileData;
        if (!GetFile(BodyStructPath, fileData))
        {
            return;
        }
        INIReader Loader(reinterpret_cast<char*>(fileData.data()), fileData.size());

        int NumMoldes = Loader.GetInteger("INIT", "Moldes", 0);
        mBodyStructData.resize(NumMoldes);
        for (int i = 0; i < NumMoldes; ++i) {
            std::string MoldeKey = "Molde" + std::to_string(i + 1);

            auto& mold = mBodyStructData[i];

            mold.x = Loader.GetInteger(MoldeKey, "X", 0);
            mold.y = Loader.GetInteger(MoldeKey, "Y", 0);
            mold.Width = Loader.GetInteger(MoldeKey, "Width", 0);
            mold.Height = Loader.GetInteger(MoldeKey, "Height", 0);
            mold.DirCount[0] = Loader.GetInteger(MoldeKey, "Dir1", 0);
            mold.DirCount[1] = Loader.GetInteger(MoldeKey, "Dir2", 0);
            mold.DirCount[2] = Loader.GetInteger(MoldeKey, "Dir3", 0);
            mold.DirCount[3] = Loader.GetInteger(MoldeKey, "Dir4", 0);
            mold.TotalGrhs = mold.DirCount[0] + mold.DirCount[1] + mold.DirCount[2] + mold.DirCount[3] + 4;
        }
    }

    void ResourceLoader::LoadHelm() {

        int16_t n = 0;
        int i = 0;
        int16_t HelmCount = 0;
        std::vector<HeadIndex> headData;
        Header MyHeader;
        std::vector<uint8_t> fileData;
        int currentPos = 0;
        if (!GetFile(HelmDataPath, fileData))
        {
            return;
        }
        memcpy_s(&MyHeader, sizeof(Header), fileData.data(), sizeof(Header));
        currentPos += sizeof(MyHeader);
        memcpy_s(&HelmCount, sizeof(int16_t), fileData.data() + currentPos, sizeof(int16_t));
        currentPos += sizeof(int16_t);
       
        // Resize array
        headData.resize(HelmCount);
        mHelmAnimData.resize(HelmCount);
        memcpy_s(headData.data(), sizeof(HeadIndex) * HelmCount, fileData.data() + currentPos, fileData.size() - currentPos);
        for (i = 0; i < HelmCount; i++) {
            for (int j = 0; j < 4; j++) headData[i].Head[j] = headData[i].Head[j] - 1;
            if (headData[i].Head[0] >= 0)
            {                
                InitGrh(mHelmAnimData[i].Head[0], headData[i].Head[0], 0);
                InitGrh(mHelmAnimData[i].Head[1], headData[i].Head[1], 0);
                InitGrh(mHelmAnimData[i].Head[2], headData[i].Head[2], 0);
                InitGrh(mHelmAnimData[i].Head[3], headData[i].Head[3], 0);
            }
            else
            {
                for (int j = 0; j < 4; j++) mHelmAnimData[i].Head[j].GrhIndex = headData[i].Head[j];
            }
        }
    }

    void ResourceLoader::LoadBodies()
    {
        std::vector<uint8_t> fileData;
        if (!GetFile(BodyDataPath, fileData))
        {
            return;
        }
        INIReader Loader(reinterpret_cast<char*>(fileData.data()), fileData.size());
        int i, LastGrh, AnimStart, x, y, FileNum;
        int j, Heading, Std;
        int k, NumCuerpos;
        float AnimSpeed;
        string BodyKey;


        NumCuerpos = Loader.GetInteger("INIT", "NumBodies", 0);

        // Resize array
        mBodyData.resize(NumCuerpos);

        for (i = 0; i < NumCuerpos; i++) {
            BodyKey = "BODY" + to_string(i+1);

            Std = Loader.GetInteger(BodyKey, "Std", 0) - 1;

            mBodyData[i].BodyOffset.X = Loader.GetInteger(BodyKey, "BodyOffsetX", 0);
            mBodyData[i].BodyOffset.Y = Loader.GetInteger(BodyKey, "BodyOffsetY", 0);
            mBodyData[i].HeadOffset.X = Loader.GetInteger(BodyKey, "HeadOffsetX", 0) + mBodyData[i].BodyOffset.X;
            mBodyData[i].HeadOffset.Y = Loader.GetInteger(BodyKey, "HeadOffsetY", 0) + mBodyData[i].BodyOffset.Y;

            if (Std < 0) {
                InitGrh(mBodyData[i].Walk[0], Loader.GetInteger(BodyKey, "Walk1", 0) - 1, 0);
                InitGrh(mBodyData[i].Walk[1], Loader.GetInteger(BodyKey, "Walk2", 0) - 1, 0);
                InitGrh(mBodyData[i].Walk[2], Loader.GetInteger(BodyKey, "Walk3", 0) - 1, 0);
                InitGrh(mBodyData[i].Walk[3], Loader.GetInteger(BodyKey, "Walk4", 0) - 1, 0);
            }
            else {
                FileNum = Loader.GetInteger(BodyKey, "FileNum", 0);
                AnimSpeed = Loader.GetInteger(BodyKey, "Speed", 0);

                if (AnimSpeed == 0) {
                    AnimSpeed = 1;
                }

                AnimSpeed = 1 / AnimSpeed / 0.018;

                LastGrh = mGrhData.size();

                // Agrego espacio para meter el body en GrhData
                mGrhData.resize(LastGrh + mBodyStructData[Std].TotalGrhs);

                auto MaxGrh = mGrhData.size() - 1;

                x = mBodyStructData[Std].x;
                y = mBodyStructData[Std].y;
                for (j = 0; j < 4; j++)
                {
                    AnimStart = LastGrh;

                    for (k = 0; k < mBodyStructData[Std].DirCount[j]; k++)
                    {
                        InitGrhWithBody(mGrhData[LastGrh], mBodyStructData[Std], FileNum, x, y, LastGrh);
                        LastGrh++;
                        x += mBodyStructData[Std].Width;
                    }
                    x = mBodyStructData[Std].x;
                    y += mBodyStructData[Std].Height;
                    Heading = mBodyHeading[j];
                    {
                        auto& grh = mGrhData[LastGrh];
                        grh.NumFrames = mBodyStructData[Std].DirCount[j];
                        grh.speed = grh.NumFrames * AnimSpeed;
                        grh.Frames.resize(grh.NumFrames);
                        for (int k = 0; k < grh.NumFrames; k++)
                        {
                            grh.Frames[k] = AnimStart + k;
                        }
                        grh.pixelWidth = mGrhData[grh.Frames[0]].pixelWidth;
                        grh.pixelHeight = mGrhData[grh.Frames[0]].pixelHeight;
                        grh.TileWidth = mGrhData[grh.Frames[0]].TileWidth;
                        grh.TileHeight = mGrhData[grh.Frames[0]].TileHeight;

                        InitGrh(mBodyData[i].Walk[Heading], LastGrh, 0);
                        LastGrh++;
                    }
                }
            }

        }
    }

    void ResourceLoader::LoadWeaponAnimations() {
        std::vector<uint8_t> fileData;
        if (!GetFile(WeaponDataPath, fileData))
        {
            return;
        }
        INIReader Loader(reinterpret_cast<char*>(fileData.data()), fileData.size());
        int i = 0;
        int k = 0;
        std::string ArmaKey = "";
        int Std = 0;
        int NumCuerpos = 0;
        int LastGrh = 0;
        int AnimStart = 0;
        int x = 0;
        int y = 0;
        int FileNum = 0;
        std::string Arch = "";

        int NumWeaponAnims = Loader.GetInteger("INIT", "NumArmas", 0);
        mWeaponAnimData.resize(NumWeaponAnims);
        for (int wI = 0; wI < NumWeaponAnims; wI++) {
            ArmaKey = "ARMA" + std::to_string(wI + 1);
            Std = Loader.GetInteger(ArmaKey, "Std", 0) - 1;
            if (Std < 0) {
                InitGrh(mWeaponAnimData[wI].WeaponWalk[0], Loader.GetInteger(ArmaKey, "Dir1", 0) -1, 0);
                InitGrh(mWeaponAnimData[wI].WeaponWalk[1], Loader.GetInteger(ArmaKey, "Dir2", 0) -1, 0);
                InitGrh(mWeaponAnimData[wI].WeaponWalk[2], Loader.GetInteger(ArmaKey, "Dir3", 0) -1, 0);
                InitGrh(mWeaponAnimData[wI].WeaponWalk[3], Loader.GetInteger(ArmaKey, "Dir4", 0) -1, 0);
            }
            else {
                FileNum = Loader.GetInteger(ArmaKey, "FileNum", 0);
                LastGrh = mGrhData.size();
                mGrhData.resize(LastGrh + mBodyStructData[Std].TotalGrhs);
                int MaxGrh = mGrhData.size() - 1;
                x = mBodyStructData[Std].x;
                y = mBodyStructData[Std].y;
                for (int j = 0; j < 4; j++) {
                    AnimStart = LastGrh;
                    for (int k = 0; k < mBodyStructData[Std].DirCount[j]; k++) {
                        InitGrhWithBody(mGrhData[LastGrh], mBodyStructData[Std], FileNum, x, y, LastGrh);
                        LastGrh++;
                        x += mBodyStructData[Std].Width;
                    }
                    x = mBodyStructData[Std].x;
                    y += mBodyStructData[Std].Height;
                    auto Heading = mBodyHeading[j];
                    {
                        auto& grh = mGrhData[LastGrh];
                        grh.NumFrames = mBodyStructData[Std].DirCount[j];
                        grh.speed = grh.NumFrames * 0.018;
                        grh.Frames.resize(grh.NumFrames);
                        for (int k = 0; k < grh.NumFrames; k++)
                        {
                            grh.Frames[k] = AnimStart + k;
                        }
                        grh.pixelWidth = mGrhData[grh.Frames[0]].pixelWidth;
                        grh.pixelHeight = mGrhData[grh.Frames[0]].pixelHeight;
                        grh.TileWidth = mGrhData[grh.Frames[0]].TileWidth;
                        grh.TileHeight = mGrhData[grh.Frames[0]].TileHeight;

                        InitGrh(mWeaponAnimData[wI].WeaponWalk[Heading], LastGrh, 0);
                        LastGrh++;
                    }
                }
            }
        }
    }

    void ResourceLoader::LoadShields()
    {
        std::vector<uint8_t> fileData;
        if (!GetFile(ShieldDataPath, fileData))
        {
            return;
        }
        INIReader Loader(reinterpret_cast<char*>(fileData.data()), fileData.size());
        int i = 0;
        int k = 0;
        std::string ElentKey = "";
        int Std = 0;
        int NumCuerpos = 0;
        int LastGrh = 0;
        int AnimStart = 0;
        int x = 0;
        int y = 0;
        int FileNum = 0;
        std::string Arch = "";

        int ItemCount = Loader.GetInteger("INIT", "NumEscudos", 0);
        mShieldAnimData.resize(ItemCount);
        for (int wI = 0; wI < ItemCount; wI++) {
            ElentKey = "ESC" + std::to_string(wI + 1);
            Std = Loader.GetInteger(ElentKey, "Std", 0) - 1;
            if (Std < 0) {
                InitGrh(mShieldAnimData[wI].Heading[0], Loader.GetInteger(ElentKey, "Dir1", 0) - 1, 0);
                InitGrh(mShieldAnimData[wI].Heading[1], Loader.GetInteger(ElentKey, "Dir2", 0) - 1, 0);
                InitGrh(mShieldAnimData[wI].Heading[2], Loader.GetInteger(ElentKey, "Dir3", 0) - 1, 0);
                InitGrh(mShieldAnimData[wI].Heading[3], Loader.GetInteger(ElentKey, "Dir4", 0) - 1, 0);
            }
            else {
                FileNum = Loader.GetInteger(ElentKey, "FileNum", 0);
                LastGrh = mGrhData.size();
                mGrhData.resize(LastGrh + mBodyStructData[Std].TotalGrhs);
                int MaxGrh = mGrhData.size() - 1;
                x = mBodyStructData[Std].x;
                y = mBodyStructData[Std].y;
                for (int j = 0; j < 4; j++) {
                    AnimStart = LastGrh;
                    for (int k = 0; k < mBodyStructData[Std].DirCount[j]; k++) {
                        InitGrhWithBody(mGrhData[LastGrh], mBodyStructData[Std], FileNum, x, y, LastGrh);
                        LastGrh++;
                        x += mBodyStructData[Std].Width;
                    }
                    x = mBodyStructData[Std].x;
                    y += mBodyStructData[Std].Height;
                    auto Heading = mBodyHeading[j];
                    {
                        auto& grh = mGrhData[LastGrh];
                        grh.NumFrames = mBodyStructData[Std].DirCount[j];
                        grh.speed = grh.NumFrames * 0.018;
                        grh.Frames.resize(grh.NumFrames);
                        for (int k = 0; k < grh.NumFrames; k++)
                        {
                            grh.Frames[k] = AnimStart + k;
                        }
                        grh.pixelWidth = mGrhData[grh.Frames[0]].pixelWidth;
                        grh.pixelHeight = mGrhData[grh.Frames[0]].pixelHeight;
                        grh.TileWidth = mGrhData[grh.Frames[0]].TileWidth;
                        grh.TileHeight = mGrhData[grh.Frames[0]].TileHeight;

                        InitGrh(mShieldAnimData[wI].Heading[Heading], LastGrh, 0);
                        LastGrh++;
                    }
                }
            }
        }
    }

    std::string GetCompressedPath(const std::string& fileName)
    {
        return std::string("OUTPUT/") + fileName;
    }

    bool ResourceLoader::GetFile(const char* fileName, std::vector<uint8_t>& fileData)
    {
        if (mCompressed)
        {
            auto paths = splitString(fileName, '/');
            auto it = mCompressedFiles.find(paths[0]);
            if (it == mCompressedFiles.end())
            {
                mCompressedFiles.insert(std::make_pair(paths[0], std::make_unique<Compressor>()));
                it = mCompressedFiles.find(paths[0]);
                auto filePath = GetCompressedPath(paths[0]);
                it->second->Open( GetFilePath(filePath.c_str()).u8string().c_str(),
                                "ht5PutasTdyRk6BSJcucumelo234583013lalivn2FRjYYBzPhnMrkmUfLMgm4TDX");
            }
            it->second->GetFileData(paths[1].c_str(), fileData);
            return true;
        }
        else
        {
            auto path = GetFilePath(fileName).u8string();
            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (!file.is_open())
            {
                return false;
            }
            file.seekg(0, std::ios::end);
            std::streampos file_size = file.tellg();
            file.seekg(0, std::ios::beg);
            fileData.resize(file_size);
            file.read(reinterpret_cast<char*>(fileData.data()), file_size);
            return true;
        }
    }

    void ResourceLoader::UnloadCompressedFiles()
    {
    }

    void ResourceLoader::StartLoading(bool compressed)
    {
        mCompressed = compressed;
        mLoadingThread = std::thread([this]() {
            LoadGrhIni();
            LoadBodyStruct();
            LoadHeads();
            LoadHelm();
            LoadBodies();
            LoadWeaponAnimations();
            LoadShields();
            LoadLocalIndex();
            });
    }

    void ResourceLoader::GetBodyInfo(CharacterRenderInfo& charInfo, int bodyIndex, int headIndex, int helmIndex, int shieldIndex, int weaponIndex)
    {
        if (mLoadingThread.joinable()) mLoadingThread.join();
        memset(&charInfo, 0, sizeof(charInfo));
        SetGrhDetails(charInfo.Body.image, mBodyData[bodyIndex-1].Walk[E_Heading_SOUTH].GrhIndex);
        charInfo.Body.HeadOffset.Y = mBodyData[bodyIndex - 1].HeadOffset.Y;
        charInfo.Body.HeadOffset.X = mBodyData[bodyIndex - 1].HeadOffset.X;
        if (headIndex > 0)
        {
            SetGrhDetails(charInfo.Head, mHeadData[headIndex - 1].Head[E_Heading_SOUTH].GrhIndex);
        }
        if (helmIndex > 0)
        {
            SetGrhDetails(charInfo.Helm, mHelmAnimData[helmIndex - 1].Head[E_Heading_SOUTH].GrhIndex);
        }
        if (weaponIndex > 0)
        {
            SetGrhDetails(charInfo.Weapon, mWeaponAnimData[weaponIndex - 1].WeaponWalk[E_Heading_SOUTH].GrhIndex);
        }
        if (shieldIndex > 0)
        {
            SetGrhDetails(charInfo.Shield, mShieldAnimData[shieldIndex - 1].Heading[E_Heading_SOUTH].GrhIndex);
        }
    }

    void ResourceLoader::GetHeadInfo(GrhDetails& headInfo, int headIndex)
    {
        if (mLoadingThread.joinable()) mLoadingThread.join();
        if (headIndex > 0)
        {
            SetGrhDetails(headInfo, mHeadData[headIndex - 1].Head[E_Heading_SOUTH].GrhIndex);
        }
    }

    void ResourceLoader::SetGrhDetails(GrhDetails& dest, int grhIndex)
    {
        if (grhIndex < 0)
        {
            dest.ImageNumber = -1;
            dest.StartPos.X = 0;
            dest.StartPos.Y = 0;
            dest.EndPos.X = 0;
            dest.EndPos.Y = 0;
            return;
        }
        if (mGrhData[grhIndex].Frames.size() > 1)
        {
            grhIndex = mGrhData[grhIndex].Frames[0];
        }
        dest.ImageNumber = mGrhData[grhIndex].FileNum;
        dest.StartPos.X = mGrhData[grhIndex].sX;
        dest.StartPos.Y = mGrhData[grhIndex].sY;
        dest.EndPos.X = mGrhData[grhIndex].pixelWidth;
        dest.EndPos.Y = mGrhData[grhIndex].pixelHeight;
    }

    void ResourceLoader::GetSpellData(SpellData& dest, int spellIndex)
    {
        if (spellIndex >= 0 && spellIndex < mSpellInfo.size())
        {
            dest = mSpellInfo[spellIndex];
        }
        else
        {
            dest.RequiredMana = 0;
            dest.IconIndex = 0;
            dest.RequiredSkill= 0;
            dest.RequiredStamina = 0;
            dest.IconIndex = 0;
            dest.Cooldown = 0;
        }
    }

    void ResourceLoader::GetObjectData(ObjectData& dest, int objIndex)
    {
        if (objIndex >= 0 && objIndex < mObjData.size())
        {
            dest = mObjData[objIndex];
        }
        else
        {
            dest.grhindex = 0;
        }
    }

    void ResourceLoader::GetNpcinfo(NpcInfo& destInfo, int npcIndex)
    {
        if (npcIndex >= 0 && npcIndex < mNpcInfo.size())
        {
            destInfo = mNpcInfo[npcIndex];
        }
    }


    Resources::Resources(bool compressed)
    {
        mResources = std::make_unique<ResourceLoader>();
        mResources->StartLoading(compressed);
    }

    Resources::~Resources()
    {
    }

    void Resources::GetBodyInfo(CharacterRenderInfo& charInfo, int bodyIndex, int headIndex, int helmIndex, int shieldIndex, int weaponIndex)
    {
        mResources->GetBodyInfo(charInfo, bodyIndex, headIndex, helmIndex, shieldIndex, weaponIndex);
    }

    void Resources::GetHeadInfo(GrhDetails& headInfo, int headIndex)
    {
        mResources->GetHeadInfo(headInfo, headIndex);
    }

    void Resources::GetGrhInfo(GrhDetails& grhInfo, int grhIndex)
    {
        mResources->SetGrhDetails(grhInfo, grhIndex - 1);
    }

    void Resources::GetSpellDetails(SpellData& spellInfo, int spellIndex)
    {
        mResources->GetSpellData(spellInfo, spellIndex - 1);
    }

    void Resources::GetObjectDetails(ObjectData& destObj, int itemIndex)
    {
        mResources->GetObjectData(destObj, itemIndex - 1);
    }

    void Resources::GetNpcInfo(NpcInfo& destInfo, int npcIndex)
    {
        mResources->GetNpcinfo(destInfo, npcIndex - 1);
    }

}