#pragma once
#include "marlin_client.h"
#include <optional>

class SteelSheets {
public:
    static constexpr float zOffsetMin = -2.0F;
    static constexpr float zOffsetMax = 0.0F;

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Iterate across the profiles and switch to the next calibrated.
    ///
    /// Printer use print sheet profile on the index 0 as a default so the method
    /// in the worst case iterate across entire profiles and return index 0 when
    /// not any other profile is calibrated yet.
    /// @return Index of the next calibrated profile.
    static size_t NextSheet();
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Determine if the given sheet profile is calibrated.
    ///
    /// In case the index of the given print sheet profile is bigger than the
    /// MAX_SHEETS method return false.
    /// @param[in] index Index of the sheet profile
    /// @return True when the profile is calibrated, False othewise.
    static bool IsSheetCalibrated(size_t index);
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Select the given print sheet profile as an active for the printer.
    /// @param[in] index Index of the sheet profile
    /// @retval true success
    /// @retval false index out of range
    static bool SelectSheet(size_t index);

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Reset the given print sheet profile to the uncalibrated state.
    ///
    /// @param[in] index Index of the sheet profile
    /// @retval true success
    /// @retval false index out of range
    static bool ResetSheet(size_t index);
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Returns number of calibrated sheets
    ///
    /// Printer use print sheet profile on the index 0 as a default so the method
    /// return always at least 1 calibrated profile.
    /// @return Return the count of the calibrated print sheet profiles.
    static size_t NumOfCalibrated();
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Determine the name of the current active print sheet profile.
    ///
    /// @param[out] buffer Buffer to store the print sheet profile
    /// @param[in] length Size of the given buffer.
    /// @return Number of characters written to the buffer. Number will be
    ///        always less than MAX_SHEET_NAME_LENGTH
    static size_t ActiveSheetName(char *buffer, size_t length);
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Determine the name of the given print sheet profile.
    ///
    /// @param[in] index Index of the sheet profile
    /// @param[out] buffer Buffer to store the print sheet profile
    /// @param[in] length Size of the given buffer.
    /// @return Number of characters written to the buffer. Number will be
    ///        always less than MAX_SHEET_NAME_LENGTH
    static size_t SheetName(size_t index, char *buffer, size_t length);
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Rename the given print sheet profile.
    ///
    /// @param[in] index Index of the sheet profile
    /// @param[in] buffer New name of the print sheet profile
    /// @param[in] length Size of the given name.
    /// @return Number of characters written to the buffer. Number will be
    ///        always less than MAX_SHEET_NAME_LENGTH
    static size_t RenameSheet(size_t index, char const *buffer, size_t length);
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Sets Z offset to currently selected sheet.
    ///
    /// Z offset is clamped between zOffsetMin and zOffsetMax and the unit is mm
    /// @param[in] offset  of the sheet
    /// @return True when Z offset is set, False otherwise.
    static bool SetZOffset(float offset);
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Gets offset of currently selected sheet.
    ///
    /// @return Z offset of the sheet
    static float GetZOffset();

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Get Z offset of sheet on index
    ///
    /// @param[in] index Index of the sheet profile
    /// @return Z offset of the sheet
    static float GetSheetOffset(size_t index);

private:
    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Sets Z offset for sheet
    ///
    /// Z offset is clamped between zOffsetMin and zOffsetMax
    /// @param[in] index Index of the sheet profile
    /// @param[in]  offset of the sheet
    /// @return True when successful, false if not
    static bool setSheetOffset(size_t index, float offset);
    SteelSheets &getSteelSheets();

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Determine the index of active sheet
    ///
    /// @return active sheet index
    static size_t activeSheetIndex();

    static Sheet getSheet(size_t index);
    static bool setSheet(size_t index, Sheet sheet);

    ///////////////////////////////////////////////////////////////////////////////
    /// @brief Updates probe Z offset variable in marlin
    ///
    /// @param[in] Z offset of the sheet
    static void updateMarlin(float offset);
};
