/*
 *
 *  $Id: pdf2dsource.cpp $
 *  Ginkgo CADx Project
 *
 *  Code addapted from DCMTK
 *
 *
 *  Copyright (C) 2001-2007, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  dcmdata
 *
 *  Author:  Michael Onken
 *
 *  Purpose: Base Class for plugins extracting pixel data from standard
 *           image files
 *
 *  Last Update:      $Author: onken $
 *  Update Date:      $Date: 2009-01-16 09:51:55 $
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */


#ifdef verify
#define MACRO_QUE_ESTORBA verify
#undef verify
#endif

#include "document2dcm.h"
#include <dcmtk/dcmdata/dcpxitem.h>

#include "dcmtk/dcmdata/dctk.h"
//#include "dcmtk/dcmdata/dcdebug.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"     /* for dcmtk version name */
#include "dcmtk/dcmdata/dcistrmf.h"

#include "pdf2dsource.h"
#include <main/controllers/controladorlog.h>
#include <main/entorno.h>

#ifdef MACRO_QUE_ESTORBA
#define verify MACRO_QUE_ESTORBA
#undef MACRO_QUE_ESTORBA
#endif


#include <wx/string.h>
#include <wx/filefn.h>


OFString PDF2DSource::inputFormat() const
{
        return "PDF";
}
/** Reads pixel data and corresponding attributes like rows etc. from image
	 *  file and inserts them into dataset.
	 *  @param dset - [out] The dataset to export the pixel data attributes to
	 *  @param outputTS - [out] The proposed transfex syntax of the dataset
	 *  @return EC_Normal, if successful, error otherwise
	 */
OFCondition PDF2DSource::readAndInsertSpecificTags( DcmDataset* dset,
                E_TransferSyntax& outputTS)
{
        OFCondition cond;

        //it's a secondary capture.. if conversion type is not set put  SD -> scanned document
        dset->putAndInsertString(DCM_ConversionType, "SD", false);
        dset->putAndInsertString(DCM_MIMETypeOfEncapsulatedDocument, "application/pdf", true);

        cond = dset->putAndInsertOFStringArray(DCM_SOPClassUID, UID_EncapsulatedPDFStorage);
        if (cond.bad())
                return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to insert SOP class into dataset");

        if (!dset->tagExists(DCM_Modality)) {
                cond = dset->putAndInsertOFStringArray(DCM_Modality, "DOC");
                if (cond.bad())
                        return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to insert Modality (DOC) into dataset");
        }

        if (!dset->tagExists(DCM_BurnedInAnnotation)) {
                cond = dset->putAndInsertOFStringArray(DCM_BurnedInAnnotation, "YES");
                if (cond.bad())
                        return makeOFCondition(OFM_dcmdata, 18, OF_error, "Unable to insert DCM_BurnedInAnnotation into dataset");
        }


        //document
        //read file into unsigned char
        {
                size_t fileSize = 0;
                {
                        std::ifstream file;
                        file.open(m_file.c_str(),std::ios_base::binary);
                        if(!file.is_open()) {
                                LOG_ERROR("PDF2DSource","Error opening file");
                                return makeOFCondition(OFM_dcmdata, 18, OF_error, "Error opening file");
                        }
                        //get the length of the file
                        file.seekg(0,std::ios::end);
                        fileSize = file.tellg();
                        if (fileSize %2 != 0) {
                                LOG_INFO("VideoDicomizer", "Video size is odd, padding with 0");
                                //file is odd
                                m_isTemp = true;
                                std::string m_TempFile = GNC::Entorno::Instance()->CreateGinkgoTempFile();
                                if (!wxCopyFile(FROMPATH(m_file), FROMPATH(m_TempFile))) {
                                        LOG_ERROR("PDF2DSource", "Error creating temp file");
                                }
                                std::ofstream ofile;
                                ofile.open(m_TempFile.c_str(), std::ios_base::app);
                                ofile << '\0';
                                ofile.close();
                                m_file = m_TempFile.c_str();
                                fileSize++;
                        }
                }
                DcmInputFileStreamFactory* pFactory = new DcmInputFileStreamFactory(m_file.c_str(), 0);
                DcmOtherByteOtherWord* fileTag = new DcmOtherByteOtherWord(DCM_EncapsulatedDocument);
                fileTag->setVR(EVR_OB);
                fileTag->createValueFromTempFile(pFactory, fileSize, EBO_unknown);

                dset->insert(fileTag, true);
        }


        outputTS = EXS_LittleEndianExplicit;
        return cond;
}

/** Do some completeness / validity checks. Should be called when
 *  dataset is completed and is about to be saved.
 *  @param dataset - [in] The dataset to check
 *  @return Error string if error occurs, empty string otherwise
 */
OFString PDF2DSource::isValid(DcmDataset& dset) const
{
        if (m_debug)
                printMessage(m_logStream, "I2DImgSource: Checking validity of DICOM output dataset");
        OFString dummy, err;
        OFCondition cond;
        err += checkAndInventType2Attrib(DCM_MIMETypeOfEncapsulatedDocument, &dset);

        err += checkAndInventType2Attrib(DCM_SOPInstanceUID, &dset);
        err += checkAndInventType2Attrib(DCM_ConversionType, &dset);

        return err;
}

PDF2DSource::~PDF2DSource()
{
        if (m_isTemp) {
                wxRemoveFile(FROMPATH(m_file));
        }
}
