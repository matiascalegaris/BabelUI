#include <iostream>
#include <fstream>
#include <string>
#include "Utils//FileUtils.h"
#include <vector>
#include "Utils/IniReader.h"
#include <thread>
#include "Resources.hpp"

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
        const char* BodyStructPath = "init/moldes.ini";
        const char* HelmDataPath = "init/cascos.ind";
        const char* WeaponDataPath = "init/armas.dat";
        const char* ShieldDataPath = "init/escudos.dat";

        const int TilePixelHeight = 32;
        const int TilePixelWidth = 32;
    }
#pragma pack(push, 1)

#pragma pack(push, 1)

#pragma pack(pop)
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
        void StartLoading();

        void GetBodyInfo(CharacterRenderInfo& charInfo, int bodyIndex, int headIndex, int helmIndex, int shieldIndex, int weaponIndex);
        void GetHeadInfo(GrhDetails& headInfo, int headIndex);
        void SetGrhDetails(GrhDetails& dest, int grhIndex);
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
    private:
        std::vector<GrhData> mGrhData;
        std::vector<MoldeCuerpo> mBodyStructData;
        std::vector<HeadIndex> mHeadList;
        std::vector<HeadData> mHeadData;
        std::vector<HeadData> mHelmAnimData;
        std::vector<BodyData> mBodyData;
        std::vector<WeaponAnimData> mWeaponAnimData;
        std::vector<ShieldAnimData> mShieldAnimData;
        E_Heading mBodyHeading[4];

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
        fstream f;
        long grh = 0, Frame = 0;
        char SeparadorClave = '=';
        char SeparadorGrh = '-';
        std::string CurrentLine;
        std::vector<std::string> Fields;

        // Abrimos el archivo. No uso FileManager porque obliga a cargar todo el archivo en memoria
        // y es demasiado grande. En cambio leo linea por linea y procesamos de a una.
        f.open(GetFilePath(GrhIniPath).u8string(), ios_base::in);
        // Leemos el total de Grhs
        while (std::getline(f, CurrentLine))
        {
            Fields = splitString(CurrentLine, SeparadorClave);

            // Buscamos la clave "NumGrh"
            if (Fields[0] == "NumGrh")
            {
                // Asignamos el tamano al array de Grhs
                auto MaxGrh = std::stoi(Fields[1]);

                mGrhData.resize(MaxGrh);
                break;
            }
        }
        // Chequeamos si pudimos leer la cantidad de Grhs
        if (mGrhData.size() <= 0) return;
        // Buscamos la posicion del primer Grh
        while (std::getline(f, CurrentLine))
        {
            if (to_uppercase(CurrentLine) == "[GRAPHICS]") // Buscamos el nodo "[Graphics]"
            {
                break;
            }
        }

        // Recorremos todos los Grhs
        while (std::getline(f, CurrentLine))
        {
            Frame = 0;
            // Ignoramos lineas vacias
            if (CurrentLine == "") continue;
            // Divimos por el "="
            Fields = splitString(CurrentLine, SeparadorClave);
            // Leemos el numero de Grh (el numero a la derecha de la palabra "Grh")
            grh = std::stol(Fields[0].substr(3)) - 1;
            // Leemos los campos de datos del Grh
            Fields = splitString(Fields[1], SeparadorGrh);
            auto& gd = mGrhData[grh];
            // Primer lugar: cantidad de frames.
            gd.NumFrames = std::stoi(Fields[0]);
            gd.Frames.resize(gd.NumFrames);
            // Tiene mas de un frame entonces es una animacion
            if (gd.NumFrames > 1)
            {
                // Segundo lugar: Leemos los numeros de grh de la animacion
                for (Frame = 1; Frame <= gd.NumFrames; Frame++)
                {
                    gd.Frames[Frame - 1] = std::stoi(Fields[Frame]) - 1;
                    if (gd.Frames[Frame - 1] <= 0 || gd.Frames[Frame - 1] > mGrhData.size()) return;
                }

                // Tercer lugar: leemos la velocidad de la animacion
                gd.speed = std::stoi(Fields[Frame]);
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

                // Segundo lugar: NumeroDelGrafico.bmp, pero sin el ".bmp"
                gd.FileNum = std::stoi(Fields[1]);

                // Tercer Lugar: La coordenada X del grafico
                gd.sX = std::stoi(Fields[2]);

                // Cuarto Lugar: La coordenada Y del grafico
                gd.sY = std::stoi(Fields[3]);

                // Quinto lugar: El ancho del grafico
                gd.pixelWidth = std::stoi(Fields[4]);

                // Sexto lugar: La altura del grafico
                gd.pixelHeight = std::stoi(Fields[5]);

                //Calculamos el ancho y alto en tiles
                gd.TileWidth = gd.pixelWidth / TilePixelWidth;
                gd.TileHeight = gd.pixelHeight / TilePixelHeight;
            }
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
        fstream f;
        f.open(GetFilePath(HeadDataPath).u8string(), ios_base::in | ios_base::binary);
        f.read(buf, sizeof(Header));

        // num de cabezas
        f.read(reinterpret_cast<char*>(&Numheads), sizeof(int16_t));

        // Resize array
        mHeadData.resize(Numheads);
        mHeadList.resize(Numheads);
        f.read(reinterpret_cast<char*>(mHeadList.data()), sizeof(HeadIndex) * Numheads);

        bool complete = f.eof();
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

    void ResourceLoader::LoadBodyStruct()
    {
        mBodyHeading[0] = E_Heading::E_Heading_SOUTH;
        mBodyHeading[1] = E_Heading_NORTH;
        mBodyHeading[2] = E_Heading::E_Heading_WEST;
        mBodyHeading[3] = E_Heading::E_Heading_EAST;

        INIReader Loader(GetFilePath(BodyStructPath).u8string());

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

        std::ifstream file(GetFilePath(HelmDataPath).u8string(), std::ios::in | std::ios::binary);
        // header
        file.read(reinterpret_cast<char*>(&MyHeader), sizeof(MyHeader));

        // head count
        file.read(reinterpret_cast<char*>(&HelmCount), sizeof(HelmCount));

        // Resize array
        headData.resize(HelmCount);
        mHelmAnimData.resize(HelmCount);
        file.read(reinterpret_cast<char*>(headData.data()), sizeof(HeadIndex) * HelmCount);
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
        file.close();
    }

    void ResourceLoader::LoadBodies()
    {
        INIReader Loader(GetFilePath(BodyDataPath).u8string());
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
        INIReader Loader(GetFilePath(WeaponDataPath).u8string());
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
        INIReader Loader(GetFilePath(ShieldDataPath).u8string());
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

    void ResourceLoader::StartLoading()
    {
        mLoadingThread = std::thread([this]() {
            LoadGrhIni();
            LoadBodyStruct();
            LoadHeads();
            LoadHelm();
            LoadBodies();
            LoadWeaponAnimations();
            LoadShields();
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


    Resources::Resources()
    {
        mResources = std::make_unique<ResourceLoader>();
        mResources->StartLoading();
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

    }

}