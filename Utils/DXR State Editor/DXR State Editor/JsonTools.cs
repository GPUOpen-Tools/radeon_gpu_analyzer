using System;
using System.Collections.Generic;
using System.Text.Encodings.Web;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Windows;
using System.Windows.Controls;
namespace DXR_State_Editor
{
    public struct ProcessedTargetText
    {
        public ProcessedTargetText(String ep, String ln)
        {
            if (ep != null)
            {
                entryPointName = ep.Trim(' ');
            }
            else
            {
                entryPointName = "";
            }
            if (ln != null)
            {
                linkName = ln.Trim(' ');
            }
            else
            {
                linkName = "";
            }
        }
        public String entryPointName;
        public String linkName;
    };
    public partial class MainWindow : Window
    {
        public class DXR_JSON_STATE__RaytracingShaderConfig : DXR_JSON_ELEMENT
        {
            public int MaxPayloadSizeInBytes { get; set; }
            public int MaxAttributeSizeInBytes { get; set; }
            public IList<String> Exports { get; set; }
        }
        public class DXR_JSON_STATE__RaytracingPipelineConfig : DXR_JSON_ELEMENT
        {
            public int MaxTraceRecursionDepth { get; set; }
            // Reinstate if RaytracingPipeline->Flags is re-added
            //public IList<String> Flags { get; set; }
            public IList<String> Exports { get; set; }
        }
        public class DXR_JSON_STATE__RootSignature : DXR_JSON_ELEMENT
        {
            public String Type { get; set; }
            public String FilePath { get; set; }
            public String Name { get; set; }
            public IList<String> Exports { get; set; }
        }
        public class DXR_JSON_STATE__HitGroup : DXR_JSON_ELEMENT
        {
            public String Type { get; set; }
            public String Name { get; set; }
            public String IntersectionShader { get; set; }
            public String AnyHitShader { get; set; }
            public String ClosestHitShader { get; set; }
        }
        public class DXR_JSON_STATE__Shader__EntryPoints : DXR_JSON_ELEMENT
        {
            public String ExportName { get; set; }
            public String LinkageName { get; set; }
        }
        public class DXR_JSON_STATE__Shader : DXR_JSON_ELEMENT
        {
            public String Type { get; set; }
            public String FilePath { get; set; }
            public IList<DXR_JSON_STATE__Shader__EntryPoints> EntryPoints { get; set; }
        }
        public class DXR_JSON_STATE : DXR_JSON_ELEMENT
        {
            // Note: force to 1.0 for now
            public String SchemaVersion { get; set; }
            public IList<DXR_JSON_STATE__Shader> Shaders { get; set; }
            public IList<DXR_JSON_STATE__HitGroup> HitGroups { get; set; }
            public IList<DXR_JSON_STATE__RootSignature> LocalRootSignatures { get; set; }
            public IList<DXR_JSON_STATE__RootSignature> GlobalRootSignatures { get; set; }
            public DXR_JSON_STATE__RaytracingPipelineConfig RaytracingPipelineConfig { get; set; }
            public IList<DXR_JSON_STATE__RaytracingShaderConfig> RaytracingShaderConfig { get; set; }
        }
        public class DXR_JSON_STATE_ROOT : DXR_JSON_ELEMENT
        {
            public DXR_JSON_STATE DXRState { get; set; }
        }
        public class DXR_JSON_ELEMENT
        {
            [JsonIgnore]
            public String macroElementParentID { get; set; }
        }
        private List<DXR_JSON_STATE__Shader> Master_ShadersByStringID = new List<DXR_JSON_STATE__Shader>();
        private List<DXR_JSON_STATE__HitGroup> Master_HitGroupsByStringID = new List<DXR_JSON_STATE__HitGroup>();
        private List<DXR_JSON_STATE__RootSignature> Master_LocalRootSignaturesByStringID = new List<DXR_JSON_STATE__RootSignature>();
        private List<DXR_JSON_STATE__RootSignature> Master_GlobalRootSignaturesByStringID = new List<DXR_JSON_STATE__RootSignature>();
        private DXR_JSON_STATE__RaytracingPipelineConfig Master_RaytracingPipelineConfig = new DXR_JSON_STATE__RaytracingPipelineConfig();
        private List<DXR_JSON_STATE__RaytracingShaderConfig> Master_ShaderPipelineConfigByStringID = new List<DXR_JSON_STATE__RaytracingShaderConfig>();
        public struct JsonFileData
        {
            public String filePath;
            public String fileContents;
            public DXR_JSON_STATE_ROOT jsonRoot;
        }
        JsonSerializerOptions jsonOptions = new JsonSerializerOptions()
        {
            Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
            WriteIndented = true,
            IgnoreNullValues = false,
            AllowTrailingCommas = false
        };
        // Note: populated in MainWindow.xaml.cs::MainWindow()
        public static List<Char> invalidFilePathChars = new List<Char>();
        public static List<Char> invalidFileNameChars = new List<Char>();
        public bool QueryJsonForNonDefaultAssociations()
        {
            bool nonDefaultJsonAssociationFound = false;
            foreach (MacroElement macroElement in Master_MacroElementList)
            {
                if (macroElement.applicationMode == Mode.Welcome ||
                    macroElement.applicationMode == Mode.GeneralStateConfig ||
                    macroElement.applicationMode == Mode.Output ||
                    macroElement.applicationMode == Mode.ApplicationError)
                {
                    continue;
                }
                if (CheckJSONAssociationByElementIDAndApplicationMode(macroElement.rootElement.Tag as String,
                    macroElement.applicationMode))
                {
                    nonDefaultJsonAssociationFound = true;
                    break;
                }
            }
            return nonDefaultJsonAssociationFound;
        }
        public String JSONInputIsValid()
        {
            String retString = "";
            int error_number = 1;
            // Validate required Shader fields
            foreach (DXR_JSON_STATE__Shader shader in Master_ShadersByStringID)
            {
                if (!(shader != null &&
                    (shader.FilePath != null && shader.FilePath.Length > 0)))
                {
                    retString += error_number +
                        ". The 'File Path' field is required for all Shader components.\n";
                    error_number++;
                    break;
                }
                if (InvalidShaderFilePathTextBoxesExist())
                {
                    retString += error_number +
                        ". One or more 'File Path' fields have invalid input.\n";
                    error_number++;
                }
            }
            if (InvalidShaderEntryPointTextBoxesExist())
            {
                retString += error_number +
                        ". One or more 'Entry Point' fields have invalid input.\n";
                error_number++;
            }
            // Validate required HitGroup fields
            foreach (DXR_JSON_STATE__HitGroup hitgroup in Master_HitGroupsByStringID)
            {
                if (!(hitgroup != null &&
                    (hitgroup.Name != null && hitgroup.Name.Length > 0)))
                {
                    retString += error_number +
                        ". The 'Name' field is required for all Hit Group components.\n";
                    error_number++;
                    break;
                }
            }
            // Validate required LocalRootSignature fields
            foreach (DXR_JSON_STATE__RootSignature localRootSignature in Master_LocalRootSignaturesByStringID)
            {
                if (!(localRootSignature != null &&
                    (localRootSignature.FilePath != null && localRootSignature.FilePath.Length > 0)))
                {
                    retString += error_number +
                        ". The 'File Path' field is required for all Local Root Signature components.\n";
                    error_number++;
                    break;
                }
            }
            // Validate required GlobalRootSignature fields
            foreach (DXR_JSON_STATE__RootSignature globalRootSignature in Master_GlobalRootSignaturesByStringID)
            {
                if (!(globalRootSignature != null &&
                    (globalRootSignature.FilePath != null && globalRootSignature.FilePath.Length > 0)))
                {
                    retString += error_number +
                        ". The 'File Path' field is required for all Global Root Signature components.\n";
                    error_number++;
                    break;
                }
            }
            // For the following, we must validate the textbox/combobox input widgets, not the JSON itself,
            // as when the textboxes/comboboxes are null they are not represented in the JSON master lists.
            // Validate required RaytracingPipeline fields
            TextBox targetRayTracingTextBox = FindName("RaytracingPipelineMaxTraceRecursionDepthTextBox0") as TextBox;
            if (!(targetRayTracingTextBox != null &&
                targetRayTracingTextBox.Text != null &&
                targetRayTracingTextBox.Text.Length > 0 &&
                int.TryParse(targetRayTracingTextBox.Text, out _)))
            {
                retString += error_number +
                    ". The integer 'Max Recursion depth' field is required for all Raytracing Pipeline components.\n";
                error_number++;
            }
            // Validate required ShaderPipeline fields
            bool invalidPayloadSizeFound = false;
            bool invalidMaxAttributeSizeFound = false;
            for (int count = 0; count < GetModeCount(Mode.ShaderPipeline); count++)
            {
                if (invalidMaxAttributeSizeFound && invalidPayloadSizeFound)
                {
                    break;
                }
                if (!invalidPayloadSizeFound)
                {
                    { // scope
                        String textBoxName = "ShaderPipelinePayloadSizeTextBox" + count;
                        TextBox targetShaderPipelineTextBox = FindName(textBoxName) as TextBox;
                        if (targetShaderPipelineTextBox == null)
                        {
                            postErrorMessage("A call to " + nameof(JSONInputIsValid) +
                                " was unable to retreive a valid registered handle for the name: " +
                                textBoxName);
                            invalidPayloadSizeFound = true;
                        }
                        if (!(targetShaderPipelineTextBox.Text != null &&
                            targetShaderPipelineTextBox.Text.Length > 0 &&
                            int.TryParse(targetShaderPipelineTextBox.Text, out _)))
                        {
                            retString += error_number +
                                ". The integer 'Payload Size' field is required for all Shader Pipeline components.\n";
                            error_number++;
                            invalidPayloadSizeFound = true;
                        }
                    }
                }
                if (!invalidMaxAttributeSizeFound)
                {
                    { // scope
                        String textBoxName = "ShaderPipelinesMaxAttributeSizeTextBox" + count;
                        TextBox targetShaderPipelineTextBox = FindName(textBoxName) as TextBox;
                        if (targetShaderPipelineTextBox == null)
                        {
                            postErrorMessage("A call to " + nameof(JSONInputIsValid) +
                                " was unable to retreive a valid registered handle for the name: " +
                                textBoxName);
                            invalidMaxAttributeSizeFound = true;
                        }
                        if (!(targetShaderPipelineTextBox.Text != null &&
                            targetShaderPipelineTextBox.Text.Length > 0 &&
                            int.TryParse(targetShaderPipelineTextBox.Text, out _)))
                        {
                            retString += error_number +
                                ". The integer 'Max Attribute Size' field is required for all Shader Pipeline components.\n";
                            error_number++;
                            invalidMaxAttributeSizeFound = true;
                        }
                    }
                }
            }
            return retString;
        }
        public List<String> CleanTargetText(String targetText)
        {
            try
            {
                List<String> exports = new List<String>();
                if (targetText == null || targetText.Length < 1)
                {
                    return null;
                }
                List<String> targetTextAsStringArray =
                    new List<String>(targetText.Split(new String[] { "," }, StringSplitOptions.None));
                foreach (String str in targetTextAsStringArray)
                {
                    if (str != null &&
                        str != "")
                    {
                        exports.Add(str.Trim());
                    }
                }
                if (exports.Count < 1)
                {
                    return null;
                }
                return exports;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to clean the export target text: " +
                    ex.ToString() + ".");
                return null;
            }
        }

        public bool VerifyComboBoxType(ComboBox targetComboBox, String typeValue)
        {
            try
            {
                bool shouldContinue = true;
                if (targetComboBox == null)
                {
                    postErrorMessage("A call to " + nameof(VerifyComboBoxType) +
                        " was unable to acquire the target ComboBoxItem for sub element");
                    shouldContinue = false;
                }

                if (shouldContinue && typeValue == null)
                {
                    postErrorMessage("A call to " + nameof(VerifyComboBoxType) +
                        " was unable to acquire the target ComboBoxItem for sub element");
                    shouldContinue = false;
                }

                bool typeIsValid = false;
                foreach (ComboBoxItem cbItem in targetComboBox.Items)
                {
                    if (shouldContinue && cbItem == null)
                    {
                        postErrorMessage("A call to " + nameof(VerifyComboBoxType) +
                        " was unable to acquire the target ComboBoxItem for sub element");
                        shouldContinue = false;
                    }

                    String cbItemString = cbItem.Content.ToString() as String;
                    if (shouldContinue && cbItemString == null)
                    {
                        postErrorMessage("A call to " + nameof(VerifyComboBoxType) +
                        " was unable to acquire the target ComboBoxItem content string for sub element");
                        shouldContinue = false;
                    }

                    if (shouldContinue && cbItemString == typeValue)
                    {
                        typeIsValid = true;
                        break;
                    }
                }

                return typeIsValid;
            }
            catch
            {
                postErrorMessage("An unhandled exception occured during while attempting to verify a ComboBox type.");
                return false;
            }
        }

        public void WriteDXRJSONtoViewGrid()
        {
            try
            {
                // Ensure the target text block is available.
                TextBox jsonViewText = FindName("jsonViewText") as TextBox;
                if (jsonViewText == null)
                {
                    postErrorMessage("A call to " + nameof(WriteDXRJSONtoViewGrid)
                        + " was unable to acquire the jsonViewText TextBlock target.");
                    return;
                }
                jsonViewText.Clear();
                // Build root structure.
                DXR_JSON_STATE_ROOT jsonRoot = new DXR_JSON_STATE_ROOT();
                jsonRoot.DXRState = new DXR_JSON_STATE();
                Int32 index = 0;
                List<Int32> indicesToRemove = new List<Int32>();
                // SchemaVersion
                // Note: force to 1.0 for now
                jsonRoot.DXRState.SchemaVersion = "1.0";
                // Shaders
                List<String> DXRExportComboBoxItemSourceList = new List<String>();
                jsonRoot.DXRState.Shaders = new List<DXR_JSON_STATE__Shader>();
                if (Master_ShadersByStringID.Count > 0)
                {
                    foreach (DXR_JSON_STATE__Shader shader in Master_ShadersByStringID)
                    {
                        if (shader == null)
                        {
                            index++;
                            continue;
                        }
                        if ((shader.FilePath != null && shader.FilePath.Length > 0) ||
                            (shader.Type != null && shader.Type.Length > 0) ||
                            (shader.EntryPoints != null && shader.EntryPoints.Count > 0))
                        {
                            if (shader.Type != null &&
                                (shader.Type == ConstStringBinaryDXILType ||
                                 shader.Type == ConstStringBinaryType))
                            {
                                // Note: Purposeful discrepency between GUI text representation and JSON text representation
                                shader.Type = ConstStringBinaryType;
                            }
                            else if (shader.Type != null &&
                                     shader.Type == ConstStringHLSLType)
                            {
                                shader.Type = ConstStringHLSLType;
                            }
                            else
                            {
                                postWarningMessage("Was unable to parse Shader->Type from JSON file.\n");
                            }

                            // Trim user-editable simple string members
                            if (shader.FilePath != null)
                            {
                                shader.FilePath.Trim(' ');
                            }
                            jsonRoot.DXRState.Shaders.Add(shader);
                        }
                        else
                        {
                            indicesToRemove.Add(index);
                        }
                        index++;
                    }
                }
                foreach (Int32 removalIndex in indicesToRemove)
                {
                    Master_ShadersByStringID.RemoveAt(removalIndex);
                }
                indicesToRemove.Clear();
                index = 0;
                if (jsonRoot.DXRState.Shaders.Count < 1)
                {
                    jsonRoot.DXRState.Shaders = null;
                }
                // Hit Groups
                jsonRoot.DXRState.HitGroups = new List<DXR_JSON_STATE__HitGroup>();
                if (Master_HitGroupsByStringID.Count > 0)
                {
                    foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                    {
                        if (hitGroup == null)
                        {
                            index++;
                            continue;
                        }
                        if ((hitGroup.Type != null && hitGroup.Type.Length > 0) ||
                            (hitGroup.Name != null && hitGroup.Name.Length > 0) ||
                            (hitGroup.IntersectionShader != null && hitGroup.IntersectionShader.Length > 0) ||
                            (hitGroup.ClosestHitShader != null && hitGroup.ClosestHitShader.Length > 0))
                        {
                            if (hitGroup.Type != null &&
                                hitGroup.Type == ConstStringBinaryDXILType)
                            {
                                // Note: Purposeful discrepency between GUI text representation and JSON text representation
                                hitGroup.Type = "Binary";
                            }

                            // Trim user-editable simple string members
                            if (hitGroup.AnyHitShader != null)
                            {
                                hitGroup.AnyHitShader.Trim(' ');
                            }
                            if (hitGroup.ClosestHitShader != null)
                            {
                                hitGroup.ClosestHitShader.Trim(' ');
                            }
                            if (hitGroup.IntersectionShader != null)
                            {
                                hitGroup.IntersectionShader.Trim(' ');
                            }
                            if (hitGroup.Name != null)
                            {
                                hitGroup.Name.Trim(' ');
                            }
                            jsonRoot.DXRState.HitGroups.Add(hitGroup);
                        }
                        else
                        {
                            indicesToRemove.Add(index);
                        }
                        index++;
                    }
                }
                foreach (Int32 removalIndex in indicesToRemove)
                {
                    Master_HitGroupsByStringID.RemoveAt(removalIndex);
                }
                indicesToRemove.Clear();
                index = 0;
                if (jsonRoot.DXRState.HitGroups.Count < 1)
                {
                    jsonRoot.DXRState.HitGroups = null;
                }
                // Local Root Signature
                jsonRoot.DXRState.LocalRootSignatures = new List<DXR_JSON_STATE__RootSignature>();
                if (Master_LocalRootSignaturesByStringID.Count > 0)
                {
                    foreach (DXR_JSON_STATE__RootSignature localRootSignature in Master_LocalRootSignaturesByStringID)
                    {
                        if (localRootSignature == null)
                        {
                            index++;
                            continue;
                        }
                        if ((localRootSignature.Type != null && localRootSignature.Type.Length > 0) ||
                            // Reinstate if RootSignature->Name is re-enabled
                            //(localRootSignature.Name != null && localRootSignature.Name.Length > 0) ||
                            (localRootSignature.FilePath != null && localRootSignature.FilePath.Length > 0) ||
                            (localRootSignature.Exports != null && localRootSignature.Exports.Count > 0))
                        {
                            if (localRootSignature.Type != null &&
                                localRootSignature.Type == ConstStringBinaryDXILType)
                            {
                                // Note: Purposeful discrepency between GUI text representation and JSON text representation
                                localRootSignature.Type = "Binary";
                            }

                            // Note: temporary hard-coded macro name
                            if (localRootSignature.Name == null || localRootSignature.Name.Length < 1)
                            {
                                localRootSignature.Name = "";
                            }
                            // Trim user-editable simple string members
                            if (localRootSignature.FilePath != null)
                            {
                                localRootSignature.FilePath.Trim(' ');
                            }
                            if (localRootSignature.Name != null)
                            {
                                localRootSignature.Name.Trim(' ');
                            }
                            if (localRootSignature.Exports != null)
                            {
                                int exportCount = localRootSignature.Exports.Count;
                                for (int exportIdx = 0; exportIdx < exportCount; exportIdx++)
                                {
                                    localRootSignature.Exports[exportIdx].Trim(' ');
                                }
                            }
                            jsonRoot.DXRState.LocalRootSignatures.Add(localRootSignature);
                        }
                        else
                        {
                            indicesToRemove.Add(index);
                        }
                        index++;
                    }
                }
                foreach (Int32 removalIndex in indicesToRemove)
                {
                    Master_LocalRootSignaturesByStringID.RemoveAt(removalIndex);
                }
                indicesToRemove.Clear();
                index = 0;
                if (jsonRoot.DXRState.LocalRootSignatures.Count < 1)
                {
                    jsonRoot.DXRState.LocalRootSignatures = null;
                }
                // Global Root Signature
                jsonRoot.DXRState.GlobalRootSignatures = new List<DXR_JSON_STATE__RootSignature>();
                if (Master_GlobalRootSignaturesByStringID.Count > 0)
                {
                    foreach (DXR_JSON_STATE__RootSignature globalRootSignature in Master_GlobalRootSignaturesByStringID)
                    {
                        if (globalRootSignature == null)
                        {
                            index++;
                            continue;
                        }
                        if (globalRootSignature != null &&
                            (globalRootSignature.Type != null && globalRootSignature.Type.Length > 0) ||
                            // Reinstate if RootSignature->Name is re-enabled
                            //(globalRootSignature.Name != null && globalRootSignature.Name.Length > 0) ||
                            (globalRootSignature.FilePath != null && globalRootSignature.FilePath.Length > 0) ||
                            (globalRootSignature.Exports != null && globalRootSignature.Exports.Count > 0))
                        {
                            if (globalRootSignature.Type != null &&
                                globalRootSignature.Type == ConstStringBinaryDXILType)
                            {
                                // Note: Purposeful discrepency between GUI text representation and JSON text representation
                                globalRootSignature.Type = "Binary";
                            }

                            // Note: temporary hard-coded macro name
                            if (globalRootSignature.Name == null || globalRootSignature.Name.Length < 1)
                            {
                                globalRootSignature.Name = "";
                            }
                            // Trim user-editable simple string members
                            if (globalRootSignature.FilePath != null)
                            {
                                globalRootSignature.FilePath.Trim(' ');
                            }
                            if (globalRootSignature.Name != null)
                            {
                                globalRootSignature.Name.Trim(' ');
                            }
                            if (globalRootSignature.Exports != null)
                            {
                                int exportCount = globalRootSignature.Exports.Count;
                                for (int exportIdx = 0; exportIdx < exportCount; exportIdx++)
                                {
                                    globalRootSignature.Exports[exportIdx].Trim(' ');
                                }
                            }
                            jsonRoot.DXRState.GlobalRootSignatures.Add(globalRootSignature);
                        }
                        else
                        {
                            indicesToRemove.Add(index);
                        }
                        index++;
                    }
                }
                foreach (Int32 removalIndex in indicesToRemove)
                {
                    Master_GlobalRootSignaturesByStringID.RemoveAt(removalIndex);
                }
                indicesToRemove.Clear();
                index = 0;
                if (jsonRoot.DXRState.GlobalRootSignatures.Count < 1)
                {
                    jsonRoot.DXRState.GlobalRootSignatures = null;
                }
                // Ray Tracing Shader Config
                jsonRoot.DXRState.RaytracingShaderConfig = new List<DXR_JSON_STATE__RaytracingShaderConfig>();
                if (Master_ShaderPipelineConfigByStringID.Count > 0)
                {
                    foreach (DXR_JSON_STATE__RaytracingShaderConfig RaytracingShaderConfig in Master_ShaderPipelineConfigByStringID)
                    {
                        if (RaytracingShaderConfig == null)
                        {
                            index++;
                            continue;
                        }
                        if (RaytracingShaderConfig.MaxPayloadSizeInBytes.ToString().Length > 0 ||
                            RaytracingShaderConfig.MaxAttributeSizeInBytes.ToString().Length > 0 ||
                            (RaytracingShaderConfig.Exports != null && RaytracingShaderConfig.Exports.Count > 0))
                        {
                            // Trim user-editable simple string members
                            if (RaytracingShaderConfig.Exports != null)
                            {
                                int exportCount = RaytracingShaderConfig.Exports.Count;
                                for (int exportIdx = 0; exportIdx < exportCount; exportIdx++)
                                {
                                    RaytracingShaderConfig.Exports[exportIdx].Trim(' ');
                                }
                            }
                            jsonRoot.DXRState.RaytracingShaderConfig.Add(RaytracingShaderConfig);
                        }
                        else
                        {
                            indicesToRemove.Add(index);
                        }
                        index++;
                    }
                }
                foreach (Int32 removalIndex in indicesToRemove)
                {
                    Master_ShaderPipelineConfigByStringID.RemoveAt(removalIndex);
                }
                indicesToRemove.Clear();
                index = 0;
                if (jsonRoot.DXRState.RaytracingShaderConfig.Count < 1)
                {
                    jsonRoot.DXRState.RaytracingShaderConfig = null;
                }
                // Ray Tracing Pipeline Config
                // Trim user-editable simple string members
                if (Master_RaytracingPipelineConfig != null &&
                    Master_RaytracingPipelineConfig.Exports != null)
                {
                    int exportCount = Master_RaytracingPipelineConfig.Exports.Count;
                    for (int exportIdx = 0; exportIdx < exportCount; exportIdx++)
                    {
                        Master_RaytracingPipelineConfig.Exports[exportIdx].Trim(' ');
                    }
                }
                jsonRoot.DXRState.RaytracingPipelineConfig = Master_RaytracingPipelineConfig;
                // Write JSON
                String jsonString = JsonSerializer.Serialize(jsonRoot, jsonOptions);
                jsonViewText.Text = jsonString;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to write the JSON data to the output grid: " +
                    ex.ToString() + ".");
            }
        }
        public bool CheckJSONAssociationByElementIDAndApplicationMode(String macroElementParentID, Mode mode)
        {
            try
            {
                if (mode != Mode.RaytracingPipeline && macroElementParentID == null)
                {
                    postErrorMessage("A call to " + nameof(CheckJSONAssociationByElementIDAndApplicationMode) +
                        " received a null macroElementParentID string with mode: " + mode
                        + ".");
                    return false;
                }
                bool associationFound = false;
                switch (mode)
                {
                    case Mode.Shader:
                        foreach (DXR_JSON_STATE__Shader shader in Master_ShadersByStringID)
                        {
                            if (shader == null)
                            {
                                continue;
                            }
                            if (shader.macroElementParentID == macroElementParentID)
                            {
                                associationFound = true;
                                break;
                            }
                        }
                        break;
                    case Mode.HitGroup:
                        foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                        {
                            if (hitGroup == null)
                            {
                                continue;
                            }
                            if (hitGroup.macroElementParentID == macroElementParentID)
                            {
                                associationFound = true;
                                break;
                            }
                        }
                        break;
                    case Mode.LocalRootSignature:
                        foreach (DXR_JSON_STATE__RootSignature localRootSignature in Master_LocalRootSignaturesByStringID)
                        {
                            if (localRootSignature == null)
                            {
                                continue;
                            }
                            if (localRootSignature.macroElementParentID == macroElementParentID)
                            {
                                associationFound = true;
                                break;
                            }
                        }
                        break;
                    case Mode.GlobalRootSignature:
                        foreach (DXR_JSON_STATE__RootSignature globalRootSignature in Master_GlobalRootSignaturesByStringID)
                        {
                            if (globalRootSignature == null)
                            {
                                continue;
                            }
                            if (globalRootSignature.macroElementParentID == macroElementParentID)
                            {
                                associationFound = true;
                                break;
                            }
                        }
                        break;
                    case Mode.RaytracingPipeline:
                        // This mode has default sate.
                        // Determine if the state has been altered.
                        if (Master_RaytracingPipelineConfig.Exports != null)
                        {
                            associationFound = true;
                            break;
                        }
                        if (Master_RaytracingPipelineConfig.MaxTraceRecursionDepth != 1)
                        {
                            associationFound = true;
                            break;
                        }
                        break;
                    case Mode.ShaderPipeline:
                        // This mode has default sate.
                        // Determine if the state has been altered.
                        if ((Master_ShaderPipelineConfigByStringID.Count != 1 &&
                            Master_ShaderPipelineConfigByStringID[0] != null) ||
                            Master_ShaderPipelineConfigByStringID[0].Exports != null ||
                            Master_ShaderPipelineConfigByStringID[0].MaxAttributeSizeInBytes != 8 ||
                            Master_ShaderPipelineConfigByStringID[0].MaxPayloadSizeInBytes != 16)
                        {
                            associationFound = true;
                            break;
                        }
                        break;
                    default:
                        postErrorMessage("A call to " + nameof(CheckJSONAssociationByElementIDAndApplicationMode) +
                        " received a call with an unauthorized application mode: " + mode + ".");
                        break;
                }
                return associationFound;
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to determine if there are JSON associations in the current dataset: " +
                    ex.ToString() + ".");
                return false;
            }
        }
        public void RemoveJSONDataByID(String macroElementParentID, Mode mode)
        {
            try
            {
                if (macroElementParentID == null)
                {
                    postErrorMessage("A call to " + nameof(RemoveJSONDataByID) +
                        "received a null MacroElementParentID.");
                    return;
                }
                if (mode == Mode.GeneralStateConfig ||
                    mode == Mode.Output ||
                    mode == Mode.Welcome ||
                    mode == Mode.ApplicationError)
                {
                    postErrorMessage("A call to " + nameof(RemoveJSONDataByID) +
                        "received an invalid mode indicator.");
                    return;
                }
                bool successfullyRemoved = false;
                switch (mode)
                {
                    case Mode.Shader:
                        foreach (DXR_JSON_STATE__Shader shader in Master_ShadersByStringID)
                        {
                            if (shader.macroElementParentID == macroElementParentID)
                            {
                                Master_ShadersByStringID.Remove(shader);
                                successfullyRemoved = true;
                                break;
                            }
                        }
                        break;
                    case Mode.HitGroup:
                        foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                        {
                            if (hitGroup.macroElementParentID == macroElementParentID)
                            {
                                Master_HitGroupsByStringID.Remove(hitGroup);
                                successfullyRemoved = true;
                                break;
                            }
                        }
                        break;
                    case Mode.LocalRootSignature:
                        foreach (DXR_JSON_STATE__RootSignature localRootSignature in Master_LocalRootSignaturesByStringID)
                        {
                            if (localRootSignature.macroElementParentID == macroElementParentID)
                            {
                                Master_LocalRootSignaturesByStringID.Remove(localRootSignature);
                                successfullyRemoved = true;
                                break;
                            }
                        }
                        break;
                    case Mode.GlobalRootSignature:
                        foreach (DXR_JSON_STATE__RootSignature globalRootSignature in Master_GlobalRootSignaturesByStringID)
                        {
                            if (globalRootSignature.macroElementParentID == macroElementParentID)
                            {
                                Master_GlobalRootSignaturesByStringID.Remove(globalRootSignature);
                                successfullyRemoved = true;
                                break;
                            }
                        }
                        break;
                    case Mode.ShaderPipeline:
                        foreach (DXR_JSON_STATE__RaytracingShaderConfig RaytracingShaderConfig in Master_ShaderPipelineConfigByStringID)
                        {
                            if (RaytracingShaderConfig.macroElementParentID == macroElementParentID)
                            {
                                Master_ShaderPipelineConfigByStringID.Remove(RaytracingShaderConfig);
                                successfullyRemoved = true;
                                break;
                            }
                        }
                        break;
                    default:
                        postErrorMessage("A call to " + nameof(RemoveJSONDataByID) +
                        " received a call with an unauthorized application mode: " + mode + ".");
                        break;
                }
                if (!successfullyRemoved)
                {
                    postErrorMessage("A call to " + nameof(RemoveJSONDataByID) +
                        " was unable to remove a MacroElement with mode: " + mode + ".");
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to remove JSON data by ID: " +
                    ex.ToString() + ".");
            }
        }
        //
        // Shader Element Construction
        //
        // Type
        public void AddShaderTypeByID(object sender, RoutedEventArgs e)
        {
            try
            {
                ComboBox emitter = sender as ComboBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                ComboBoxItem cbItem = emitter.SelectedItem as ComboBoxItem;
                if (cbItem == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                        " received a null ComboBoxItem.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String targetText = cbItem.Content.ToString();
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                if (CurrentApplicationMode != Mode.Shader)
                {
                    postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode + ".");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__Shader jsonShader = null;
                foreach (DXR_JSON_STATE__Shader Shader in Master_ShadersByStringID)
                {
                    if (Shader.macroElementParentID == macroElementParentID)
                    {
                        jsonShader = Shader;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonShader == null)
                {
                    jsonShader = new DXR_JSON_STATE__Shader()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonShader == null)
                    {
                        postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                            " failed to find or create a valid DXR_JSON_STATE__Shader.");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_ShadersByStringID.Add(jsonShader);
                }
                if (targetText.Length > 0)
                {
                    jsonShader.Type = targetText;
                }
                else
                {
                    jsonShader.Type = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a shader type element by ID: " +
                    ex.ToString() + ".");
            }
        }
        // Entry Point
        public List<TextBox> InvalidShaderEntryPointTextBoxes = new List<TextBox>();
        public bool InvalidShaderEntryPointTextBoxesExist()
        {
            return (InvalidShaderEntryPointTextBoxes.Count == 0) ? false : true;
        }
        public void AddShaderEntryPointByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderEntryPointByID) +
                        " received a null reference to the TextBox eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddShaderEntryPointByID) +
                        " received anull or zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderEntryPointByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.Shader)
                {
                    postErrorMessage("A call to " + nameof(AddShaderEntryPointByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode + ".");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                const String malformedEntryPointCall = "A call to " + nameof(AddShaderEntryPointByID) + "received a malfomed entry point. ";
                // Process target text.
                String postProcessedCompoundEntryPointText = "";
                List<ProcessedTargetText> processedTargetText = new List<ProcessedTargetText>();
                // Extract the contents betwen compoundEntryPoints <>. All inputs must be well-formed.
                String tmp_subString = "";
                int openingCompoundEntryPointPosition = -1;
                int closingCompoundEntryPointPosition = -1;
                Char[] charsToTrim = { '<', '>', ' ' };
                List<String> tmp_strList = new List<String>();
                HashSet<int> uniqeCompoundEntryPointPositions = new HashSet<int>();
                if (targetText.Length > 0 &&
                    targetText[0] == ' ')
                {
                    String errString = "The first character in this field cannot be whitespace.";
                    postErrorMessage(malformedEntryPointCall + errString);
                    InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                for (int strIdx = 0; strIdx < targetText.Length; strIdx++)
                {
                    tmp_strList.Clear();
                    if (targetText[strIdx] == '<')
                    {
                        openingCompoundEntryPointPosition = strIdx;
                        if (!uniqeCompoundEntryPointPositions.Add(strIdx))
                        {
                            String errString = "An opening compoundEntryPoint '<' is referenced by multiple closing compoundEntryPoints '>'.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                    }
                    else if (targetText[strIdx] == '>')
                    {
                        closingCompoundEntryPointPosition = strIdx;
                        if (closingCompoundEntryPointPosition < openingCompoundEntryPointPosition)
                        {
                            String errString = "A closing compoundEntryPoint '>' was provided before an opening compoundEntryPoint '<'.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                        if (!uniqeCompoundEntryPointPositions.Add(strIdx))
                        {
                            String errString = "A closing compoundEntryPoint '>' is referenced by multiple opening compoundEntryPoints '<'.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                        int endSnip = (closingCompoundEntryPointPosition > openingCompoundEntryPointPosition) ?
                            closingCompoundEntryPointPosition - openingCompoundEntryPointPosition : openingCompoundEntryPointPosition - closingCompoundEntryPointPosition;
                        if (openingCompoundEntryPointPosition > closingCompoundEntryPointPosition)
                        {
                            String errString = "String breaks char indexing rules.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                        if (openingCompoundEntryPointPosition < 0)
                        {
                            String errString = "String breaks char indexing rules.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                        if (closingCompoundEntryPointPosition > targetText.Length)
                        {
                            String errString = "String breaks char indexing rules.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                        tmp_subString = targetText.Substring(openingCompoundEntryPointPosition, endSnip);
                        tmp_strList.AddRange(tmp_subString.Split(','));
                        if (tmp_strList.Count != 2)
                        {
                            String errString = " A <name, linkage name> pair must be split by ',' into exactly two substrings.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                        if (strIdx < (targetText.Length - 1) &&
                            strIdx + 1 < targetText.Length &&
                            targetText[strIdx + 1] != ',')
                        {
                            String errString = "A <name, linkage name> pair must be the last element in a comma seperated list, or followed" +
                                " by a comma.";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                        // Add the processed text to the list.
                        String cleanName = tmp_strList[0].Trim(charsToTrim);
                        String cleanLinkageName = tmp_strList[1].Trim(charsToTrim);
                        if (cleanName.Length > 0 && cleanLinkageName.Length > 0)
                        {
                            processedTargetText.Add(new ProcessedTargetText(cleanName, cleanLinkageName));
                        }
                        openingCompoundEntryPointPosition = -1;
                        closingCompoundEntryPointPosition = -1;
                    }
                    else if (strIdx > 0 &&
                             targetText[strIdx] == ' ' &&
                             targetText[strIdx - 1] != ',')
                    {
                        // Explicitly allow multiple spaces
                        if (targetText[strIdx - 1] != ' ')
                        {
                            String errString = "Whtiespace must be predicated by a comma in the form \", \".";
                            postErrorMessage(malformedEntryPointCall + errString);
                            InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                            WriteDXRJSONtoViewGrid();
                            return;
                        }
                    }
                    else if (openingCompoundEntryPointPosition == -1 && closingCompoundEntryPointPosition == -1)
                    {
                        postProcessedCompoundEntryPointText += targetText[strIdx];
                    }
                }
                if (openingCompoundEntryPointPosition != -1 && closingCompoundEntryPointPosition == -1)
                {
                    String errString = "A brace '<' was opened but never closed with '>'.";
                    postErrorMessage(malformedEntryPointCall + errString);
                    InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (openingCompoundEntryPointPosition == -1 && closingCompoundEntryPointPosition != -1)
                {
                    String errString = "A brace '>' was closed but never opened with '<'.";
                    postErrorMessage(malformedEntryPointCall + errString);
                    InvalidateGeneralTextBox(emitter, errString, InvalidShaderEntryPointTextBoxes);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // From this point, the input text for Shader->EntryPoint is assumed valid.
                InvalidShaderEntryPointTextBoxes.Remove(emitter);
                // Restore proper tooltip
                emitter.ToolTip = shaderEntryPointToolTipString;
                // Add the remaining names (without linkage names) to the list.
                tmp_strList.Clear();
                tmp_strList.AddRange(postProcessedCompoundEntryPointText.Split(new String[] { "," }, StringSplitOptions.None));
                foreach (String name in tmp_strList)
                {
                    String cleanName = name.Trim(charsToTrim);
                    if (cleanName.Length > 0)
                    {
                        processedTargetText.Add(new ProcessedTargetText(cleanName, ""));
                    }
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__Shader jsonShader = null;
                foreach (DXR_JSON_STATE__Shader shader in Master_ShadersByStringID)
                {
                    if (shader.macroElementParentID == macroElementParentID)
                    {
                        jsonShader = shader;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonShader == null)
                {
                    jsonShader = new DXR_JSON_STATE__Shader()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonShader == null)
                    {
                        postErrorMessage("A call to " + nameof(AddShaderEntryPointByID) +
                            " failed to find or create a valid DXR_JSON_STATE__Shader");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_ShadersByStringID.Add(jsonShader);
                }
                //bool newEntryPointInstanceCreated = false;
                if (jsonShader.EntryPoints == null)
                {
                    jsonShader.EntryPoints = new List<DXR_JSON_STATE__Shader__EntryPoints>();
                }
                else
                {
                    jsonShader.EntryPoints.Clear();
                }
                // Add values to JSON representation
                foreach (ProcessedTargetText processedText in processedTargetText)
                {
                    if (processedText.entryPointName != null &&
                        processedText.linkName != null &&
                        (processedText.entryPointName.Length > 0 || processedText.linkName.Length > 0))
                    {
                        jsonShader.EntryPoints.Add(new DXR_JSON_STATE__Shader__EntryPoints
                        {
                            ExportName = processedText.entryPointName,
                            LinkageName = processedText.linkName,
                            macroElementParentID = macroElementParentID
                        });
                    }
                }
                if (jsonShader.EntryPoints.Count <= 0)
                {
                    jsonShader.EntryPoints = null;
                }
                // Restore default textbox style.
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a shader entry point element by ID: " +
                    ex.ToString());
            }
        }
        // File Path
        public void AddShaderFilePathByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderFilePathByID) +
                        " received a null reference to the TextBox eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddShaderFilePathByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderFilePathByID) +
                        " received a null content string.");
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (targetText.Length < 1)
                {
                    postErrorMessage("A call to " + nameof(AddShaderFilePathByID) +
                        " received an empty content string.");
                    // This field is input verified, thus we must be sure to remove the old
                    // value which may be in JSON view
                    {
                        foreach (DXR_JSON_STATE__Shader shader in Master_ShadersByStringID)
                        {
                            if (shader.macroElementParentID == macroElementParentID)
                            {
                                shader.FilePath = null;
                                break;
                            }
                        }
                    }
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String file_name = System.IO.Path.GetFileName(targetText);
                String file_path = targetText.Remove(targetText.IndexOf(file_name), file_name.Length);
                // Validate the file path
                for (int pos = 0; pos < invalidFilePathChars.Count; pos++)
                {
                    if (file_path.Contains(invalidFilePathChars[pos].ToString()))
                    {
                        String errString = "The following file path character is illegal: " + invalidFilePathChars[pos] + ".";
                        postErrorMessage("A call to " + nameof(AddShaderFilePathByID) + " received a malformed file path." + errString);
                        InvalidateGeneralTextBox(emitter, errString, InvalidShaderFilePathTextBoxes);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                }
                // Validate the file name
                for (int pos = 0; pos < invalidFileNameChars.Count; pos++)
                {
                    if (file_name.Contains(invalidFileNameChars[pos].ToString()))
                    {
                        String errString = "The following file name character is illegal: " + invalidFileNameChars[pos] + ".";
                        postErrorMessage("A call to " + nameof(AddShaderFilePathByID) + " received a malformed file name." + errString);
                        InvalidateGeneralTextBox(emitter, errString, InvalidShaderFilePathTextBoxes);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                }
                // From this point, the input text for Shader->FilePath is assumed valid.
                InvalidShaderFilePathTextBoxes.Remove(emitter);
                // Restore proper tooltip
                emitter.ToolTip = shaderFilePathToolTipString;
                if (CurrentApplicationMode != Mode.Shader)
                {
                    postErrorMessage("A call to " + nameof(AddShaderFilePathByID) +
                        " was initiated from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__Shader jsonShader = null;
                foreach (DXR_JSON_STATE__Shader shader in Master_ShadersByStringID)
                {
                    if (shader.macroElementParentID == macroElementParentID)
                    {
                        jsonShader = shader;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonShader == null)
                {
                    jsonShader = new DXR_JSON_STATE__Shader()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonShader == null)
                    {
                        postErrorMessage("A call to " + nameof(AddShaderFilePathByID) +
                            " failed to find or create a valid DXR_JSON_STATE__Shader");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_ShadersByStringID.Add(jsonShader);
                }
                if (targetText.Length > 0)
                {
                    jsonShader.FilePath = targetText;
                }
                else
                {
                    jsonShader.FilePath = null;
                }
                // Restore default textbox style.
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a shader file path element by ID: " +
                    ex.ToString());
            }
        }
        //
        // Hit Group Element Construction
        //
        // Type
        public void AddHitGroupTypeByID(object sender, RoutedEventArgs e)
        {
            try
            {
                ComboBox emitter = sender as ComboBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupTypeByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupTypeByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                ComboBoxItem cbItem = emitter.SelectedItem as ComboBoxItem;
                if (cbItem == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                        " received a null ComboBoxItem.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String targetText = cbItem.Content.ToString();
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddShaderTypeByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                if (CurrentApplicationMode != Mode.HitGroup)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupTypeByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__HitGroup jsonHitGroup = null;
                foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                {
                    if (hitGroup.macroElementParentID == macroElementParentID)
                    {
                        jsonHitGroup = hitGroup;
                        break;
                    }
                }

                // Create new JSON element if one has not been created.
                if (jsonHitGroup == null)
                {
                    jsonHitGroup = new DXR_JSON_STATE__HitGroup()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonHitGroup == null)
                    {
                        postErrorMessage("A call to " + nameof(AddHitGroupTypeByID) +
                            " failed to find or create a valid DXR_JSON_STATE__HitGroup");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_HitGroupsByStringID.Add(jsonHitGroup);
                }

                if (targetText.Length > 0)
                {
                    jsonHitGroup.Type = targetText;
                }
                else
                {
                    jsonHitGroup.Type = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a hit group type element by ID: " +
                    ex.ToString());
            }
        }
        // Name
        public void AddHitGroupNameByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupNameByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupNameByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupNameByID) +
                        " received a null content string.");
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (targetText.Length < 1)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupNameByID) +
                        " received an empty content string.");
                    // This field is input verified, thus we must be sure to remove the old
                    // value which may be in JSON view
                    {
                        foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                        {
                            if (hitGroup.macroElementParentID == macroElementParentID)
                            {
                                hitGroup.Name = null;
                                break;
                            }
                        }
                    }
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.HitGroup)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupNameByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__HitGroup jsonHitGroup = null;
                foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                {
                    if (hitGroup.macroElementParentID == macroElementParentID)
                    {
                        jsonHitGroup = hitGroup;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonHitGroup == null)
                {
                    jsonHitGroup = new DXR_JSON_STATE__HitGroup()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonHitGroup == null)
                    {
                        postErrorMessage("A call to " + nameof(AddHitGroupNameByID) +
                            " failed to find or create a valid DXR_JSON_STATE__HitGroup");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_HitGroupsByStringID.Add(jsonHitGroup);
                }
                if (targetText.Length > 0)
                {
                    jsonHitGroup.Name = targetText;
                }
                else
                {
                    jsonHitGroup.Name = null;
                }
                // Restore default text box style
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught  while attempting to add a hit group name element by ID: " +
                    ex.ToString());
            }
        }
        // Intersection Shader
        public void AddHitGroupIntersectionShaderByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupIntersectionShaderByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupIntersectionShaderByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupIntersectionShaderByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.HitGroup)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupIntersectionShaderByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__HitGroup jsonHitGroup = null;
                foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                {
                    if (hitGroup.macroElementParentID == macroElementParentID)
                    {
                        jsonHitGroup = hitGroup;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonHitGroup == null)
                {
                    jsonHitGroup = new DXR_JSON_STATE__HitGroup()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonHitGroup == null)
                    {
                        postErrorMessage("A call to " + nameof(AddHitGroupIntersectionShaderByID) +
                            " failed to find or create a valid DXR_JSON_STATE__HitGroup");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_HitGroupsByStringID.Add(jsonHitGroup);
                }
                if (targetText.Length > 0)
                {
                    jsonHitGroup.IntersectionShader = targetText;
                }
                else
                {
                    jsonHitGroup.IntersectionShader = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a hit group intersection shader element by ID: " +
                    ex.ToString());
            }
        }
        // Any Hit Shader
        public void AddHitGroupAnyHitShaderByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupAnyHitShaderByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupAnyHitShaderByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupAnyHitShaderByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.HitGroup)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupAnyHitShaderByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__HitGroup jsonHitGroup = null;
                foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                {
                    if (hitGroup.macroElementParentID == macroElementParentID)
                    {
                        jsonHitGroup = hitGroup;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonHitGroup == null)
                {
                    jsonHitGroup = new DXR_JSON_STATE__HitGroup()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonHitGroup == null)
                    {
                        postErrorMessage("A call to " + nameof(AddHitGroupAnyHitShaderByID) +
                            " failed to find or create a valid DXR_JSON_STATE__HitGroup");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_HitGroupsByStringID.Add(jsonHitGroup);
                }
                if (targetText.Length > 0)
                {
                    jsonHitGroup.AnyHitShader = targetText;
                }
                else
                {
                    jsonHitGroup.AnyHitShader = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a hit group any hit shader element by ID: " +
                    ex.ToString());
            }
        }
        // Closest Hit Shader
        public void AddHitGroupClosestHitShaderByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupClosestHitShaderByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupClosestHitShaderByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupClosestHitShaderByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.HitGroup)
                {
                    postErrorMessage("A call to " + nameof(AddHitGroupClosestHitShaderByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__HitGroup jsonHitGroup = null;
                foreach (DXR_JSON_STATE__HitGroup hitGroup in Master_HitGroupsByStringID)
                {
                    if (hitGroup.macroElementParentID == macroElementParentID)
                    {
                        jsonHitGroup = hitGroup;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonHitGroup == null)
                {
                    jsonHitGroup = new DXR_JSON_STATE__HitGroup()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonHitGroup == null)
                    {
                        postErrorMessage("A call to " + nameof(AddHitGroupClosestHitShaderByID) +
                            " failed to find or create a valid DXR_JSON_STATE__HitGroup");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_HitGroupsByStringID.Add(jsonHitGroup);
                }
                if (targetText.Length > 0)
                {
                    jsonHitGroup.ClosestHitShader = targetText;
                }
                else
                {
                    jsonHitGroup.ClosestHitShader = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a hit group closest hit shader element by ID: " +
                    ex.ToString());
            }
        }
        //
        // Local Root Signature Element Construction
        //
        // Type
        public void AddLocalRootSignatureTypeByID(object sender, RoutedEventArgs e)
        {
            try
            {
                ComboBox emitter = sender as ComboBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureTypeByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureTypeByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                ComboBoxItem cbItem = emitter.SelectedItem as ComboBoxItem;
                if (cbItem == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureTypeByID) +
                        " received a null ComboBoxItem.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String targetText = cbItem.Content.ToString();
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureTypeByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                if (CurrentApplicationMode != Mode.LocalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureTypeByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_LocalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonRootSignature == null)
                {
                    jsonRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddLocalRootSignatureTypeByID) +
                            " failed to find or create a valid DXR_JSON_STATE__HitGroup");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_LocalRootSignaturesByStringID.Add(jsonRootSignature);
                }
                if (targetText.Length > 0)
                {
                    jsonRootSignature.Type = targetText;
                }
                else
                {
                    jsonRootSignature.Type = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a local root signature type element by ID: " +
                    ex.ToString());
            }
        }
        // File Path
        public List<TextBox> InvalidShaderFilePathTextBoxes = new List<TextBox>();
        public bool InvalidShaderFilePathTextBoxesExist()
        {
            return (InvalidShaderFilePathTextBoxes.Count != 0);
        }
        public void AddLocalRootSignatureFilePathByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureFilePathByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureFilePathByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureFilePathByID) +
                        " received a null content string.");
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (targetText.Length < 1)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureFilePathByID) +
                        " received an empty content string.");
                    // This field is input verified, thus we must be sure to remove the old
                    // value which may be in JSON view
                    {
                        foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_LocalRootSignaturesByStringID)
                        {
                            if (rootSignature.macroElementParentID == macroElementParentID)
                            {
                                rootSignature.FilePath = null;
                                break;
                            }
                        }
                    }
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.LocalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureFilePathByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonLocalRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_LocalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonLocalRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonLocalRootSignature == null)
                {
                    jsonLocalRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonLocalRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddLocalRootSignatureFilePathByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_LocalRootSignaturesByStringID.Add(jsonLocalRootSignature);
                }
                if (targetText.Length > 0)
                {
                    jsonLocalRootSignature.FilePath = targetText;
                }
                else
                {
                    jsonLocalRootSignature.FilePath = null;
                }
                // Restore default text box style
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a local root signature file path element by ID: " +
                    ex.ToString());
            }
        }
        //  Macro Name Shader
        public void AddLocalRootSignatureMacroNameShaderByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureMacroNameShaderByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureMacroNameShaderByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureMacroNameShaderByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.LocalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureMacroNameShaderByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonLocalRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_LocalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonLocalRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonLocalRootSignature == null)
                {
                    jsonLocalRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonLocalRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddLocalRootSignatureMacroNameShaderByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_LocalRootSignaturesByStringID.Add(jsonLocalRootSignature);
                }
                if (targetText.Length > 0)
                {
                    jsonLocalRootSignature.Name = targetText;
                }
                else
                {
                    jsonLocalRootSignature.Name = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a local root signature macro name element by ID: " +
                    ex.ToString());
            }
        }
        // Exports
        public void AddLocalRootSignatureExportByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureExportByID) +
                        " received a null reference to the TextBox eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureExportByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureExportByID) +
                        " received a null target text string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.LocalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddLocalRootSignatureExportByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonLocalRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_LocalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonLocalRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonLocalRootSignature == null)
                {
                    jsonLocalRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonLocalRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddLocalRootSignatureMacroNameShaderByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_LocalRootSignaturesByStringID.Add(jsonLocalRootSignature);
                }
                jsonLocalRootSignature.Exports = CleanTargetText(targetText);
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a local root signature export name element by ID: " +
                    ex.ToString());
            }
        }
        //
        // Global Root Signature Element Construction
        //
        // Type
        public void AddGlobalRootSignatureTypeByID(object sender, RoutedEventArgs e)
        {
            try
            {
                ComboBox emitter = sender as ComboBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureTypeByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureTypeByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                ComboBoxItem cbItem = emitter.SelectedItem as ComboBoxItem;
                if (cbItem == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureTypeByID) +
                        " received a null ComboBoxItem.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                String targetText = cbItem.Content.ToString();
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureTypeByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                if (CurrentApplicationMode != Mode.GlobalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureTypeByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }

                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_GlobalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonRootSignature == null)
                {
                    jsonRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddGlobalRootSignatureTypeByID) +
                            " failed to find or create a valid DXR_JSON_STATE__HitGroup");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_GlobalRootSignaturesByStringID.Add(jsonRootSignature);
                }
                if (targetText.Length > 0)
                {
                    jsonRootSignature.Type = targetText;
                }
                else
                {
                    jsonRootSignature.Type = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a global root signature type element by ID: " +
                    ex.ToString());
            }
        }
        // File Path
        public void AddGlobalRootSignatureFilePathByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureFilePathByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureFilePathByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureFilePathByID) +
                        " received a null content string.");
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (targetText.Length < 1)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureFilePathByID) +
                        " received an empty content string.");
                    // This field is input verified, thus we must be sure to remove the old
                    // value which may be in JSON view
                    {
                        foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_GlobalRootSignaturesByStringID)
                        {
                            if (rootSignature.macroElementParentID == macroElementParentID)
                            {
                                rootSignature.FilePath = null;
                                break;
                            }
                        }
                    }
                    InvalidateGeneralTextBox(emitter);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.GlobalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureFilePathByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonGlobalRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_GlobalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonGlobalRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonGlobalRootSignature == null)
                {
                    jsonGlobalRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonGlobalRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddGlobalRootSignatureFilePathByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_GlobalRootSignaturesByStringID.Add(jsonGlobalRootSignature);
                }
                if (targetText.Length > 0)
                {
                    jsonGlobalRootSignature.FilePath = targetText;
                }
                else
                {
                    jsonGlobalRootSignature.FilePath = null;
                }
                // Restore default text box style
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a global root signature file path element by ID: " +
                    ex.ToString());
            }
        }
        //  Macro Name Shader
        public void AddGlobalRootSignatureMacroNameShaderByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureMacroNameShaderByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureMacroNameShaderByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureMacroNameShaderByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.GlobalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureMacroNameShaderByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonGlobalRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_GlobalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonGlobalRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonGlobalRootSignature == null)
                {
                    jsonGlobalRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonGlobalRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddGlobalRootSignatureMacroNameShaderByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_GlobalRootSignaturesByStringID.Add(jsonGlobalRootSignature);
                }
                if (targetText.Length > 0)
                {
                    jsonGlobalRootSignature.Name = targetText;
                }
                else
                {
                    jsonGlobalRootSignature.Name = null;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a global root signature name element by ID: " +
                    ex.ToString());
            }
        }
        // Exports
        public void AddGlobalRootSignatureExportByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureExportByID) +
                        " received a null reference to the TextBox eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureExportByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureExportByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.GlobalRootSignature)
                {
                    postErrorMessage("A call to " + nameof(AddGlobalRootSignatureExportByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RootSignature jsonGlobalRootSignature = null;
                foreach (DXR_JSON_STATE__RootSignature rootSignature in Master_GlobalRootSignaturesByStringID)
                {
                    if (rootSignature.macroElementParentID == macroElementParentID)
                    {
                        jsonGlobalRootSignature = rootSignature;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonGlobalRootSignature == null)
                {
                    jsonGlobalRootSignature = new DXR_JSON_STATE__RootSignature()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonGlobalRootSignature == null)
                    {
                        postErrorMessage("A call to " + nameof(AddGlobalRootSignatureMacroNameShaderByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_GlobalRootSignaturesByStringID.Add(jsonGlobalRootSignature);
                }
                jsonGlobalRootSignature.Exports = CleanTargetText(targetText);
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a global root signature exports element by ID: " +
                    ex.ToString());
            }
        }
        //
        // Raytracing Pipeline Element Construction
        //
        // Max Recursion Depth
        public void EditRaytracingPipelineMaxRecursionDepthByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineMaxRecursionDepthByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineMaxRecursionDepthByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.RaytracingPipeline)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineMaxRecursionDepthByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                try
                {
                    Int32 checked_target_text = Convert.ToInt32(targetText);
                    if (checked_target_text > 0)
                    {
                        Master_RaytracingPipelineConfig.MaxTraceRecursionDepth = checked_target_text;
                    }
                }
                catch (Exception ex)
                {
                    if (ex is FormatException)
                    {
                        postErrorMessage("A call to " + nameof(EditRaytracingPipelineMaxRecursionDepthByID) +
                          " received a request to convert an invalid string to an int. The string must be a sequence of digits.");
                        InvalidateGeneralTextBox(emitter);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    else if (ex is OverflowException)
                    {
                        postErrorMessage("A call to " + nameof(EditRaytracingPipelineMaxRecursionDepthByID) +
                          " received a request to convert an invalid string to an int." +
                          " The string must represent a number representable by a signed 32 bit integer.");
                        InvalidateGeneralTextBox(emitter);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                }
                // Restore default textbox style.
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a raytracing pipeline max recursion depth element by ID: " +
                    ex.ToString());
            }
        }
        // Flags
        public void EditRaytracingPipelineFlagsByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineFlagsByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineFlagsByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.RaytracingPipeline)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineFlagsByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                List<String> targetTextAsStringArray =
                    new List<String>(targetText.Split(new String[] { ", " }, StringSplitOptions.None));
                foreach (String str in targetTextAsStringArray.ToArray())
                {
                    if (str == "" || str == null)
                    {
                        targetTextAsStringArray.Remove(str);
                    }
                }
                // Note: restore if RaytracingPipelienConfig->Flags member is reinstated.
                //if (targetTextAsStringArray.Count > 0)
                //{
                //    Master_RaytracingPipelineConfig.Flags = targetTextAsStringArray;
                //}
                //else
                //{
                //    Master_RaytracingPipelineConfig.Flags = null;
                //}
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a raytracing pipeline flags element by ID: " +
                    ex.ToString());
            }
        }
        // Exports
        public void EditRaytracingPipelineExportsByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineExportsByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineExportsByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.RaytracingPipeline)
                {
                    postErrorMessage("A call to " + nameof(EditRaytracingPipelineExportsByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                Master_RaytracingPipelineConfig.Exports = CleanTargetText(targetText);
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a raytracing pipeline exports element by ID: " +
                    ex.ToString());
            }
        }
        //
        // Raytracing Shader Config Construction
        //
        // Max Payload Size
        public void AddRaytracingShaderConfigMaxPayloadSizeByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxPayloadSizeByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxPayloadSizeByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxPayloadSizeByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.ShaderPipeline)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxPayloadSizeByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RaytracingShaderConfig jsonRaytracingShaderConfig = null;
                foreach (DXR_JSON_STATE__RaytracingShaderConfig rayTracingShaderConfig in Master_ShaderPipelineConfigByStringID)
                {
                    if (rayTracingShaderConfig.macroElementParentID == macroElementParentID)
                    {
                        jsonRaytracingShaderConfig = rayTracingShaderConfig;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonRaytracingShaderConfig == null)
                {
                    jsonRaytracingShaderConfig = new DXR_JSON_STATE__RaytracingShaderConfig()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonRaytracingShaderConfig == null)
                    {
                        postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxPayloadSizeByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_ShaderPipelineConfigByStringID.Add(jsonRaytracingShaderConfig);
                }
                try
                {
                    Int32 checked_target_text = Convert.ToInt32(targetText);
                    if (checked_target_text > 0)
                    {
                        jsonRaytracingShaderConfig.MaxPayloadSizeInBytes = checked_target_text;
                    }
                }
                catch (Exception ex)
                {
                    if (ex is FormatException)
                    {
                        postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxPayloadSizeByID) +
                          " received a request to convert an invalid string to an int. The string must be a sequence of digits.");
                        InvalidateGeneralTextBox(emitter);
                        RemoveJSONDataByID(macroElementParentID, Mode.RaytracingPipeline);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    else if (ex is OverflowException)
                    {
                        postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxPayloadSizeByID) +
                          " received a request to convert an invalid string to an int." +
                          " The string must represent a number representable by a signed 32 bit integer.");
                        InvalidateGeneralTextBox(emitter);
                        RemoveJSONDataByID(macroElementParentID, Mode.RaytracingPipeline);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                }
                // Restore default textbox style.
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a shader pipeline max payload size element by ID: " +
                    ex.ToString());
            }
        }
        // Max Attribute Size
        public void AddRaytracingShaderConfigMaxAttributeSizeByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxAttributeSizeByID) +
                        " received a null reference to the eimitter.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxAttributeSizeByID) +
                        " received a zero length ID string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxAttributeSizeByID) +
                        " received a null content string.");
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                if (CurrentApplicationMode != Mode.ShaderPipeline)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxAttributeSizeByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    WriteDXRJSONtoViewGrid();
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RaytracingShaderConfig jsonRaytracingShaderConfig = null;
                foreach (DXR_JSON_STATE__RaytracingShaderConfig raytracingShaderConfig in Master_ShaderPipelineConfigByStringID)
                {
                    if (raytracingShaderConfig.macroElementParentID == macroElementParentID)
                    {
                        jsonRaytracingShaderConfig = raytracingShaderConfig;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonRaytracingShaderConfig == null)
                {
                    jsonRaytracingShaderConfig = new DXR_JSON_STATE__RaytracingShaderConfig()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonRaytracingShaderConfig == null)
                    {
                        postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxAttributeSizeByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RootSignature");
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_ShaderPipelineConfigByStringID.Add(jsonRaytracingShaderConfig);
                }
                try
                {
                    Int32 checked_target_text = Convert.ToInt32(targetText);
                    if (checked_target_text > 0)
                    {
                        jsonRaytracingShaderConfig.MaxAttributeSizeInBytes = checked_target_text;
                    }
                }
                catch (Exception ex)
                {
                    if (ex is FormatException)
                    {
                        postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxAttributeSizeByID) +
                          " received a request to convert an invalid string to an int. The string must be a sequence of digits.");
                        InvalidateGeneralTextBox(emitter);
                        RemoveJSONDataByID(macroElementParentID, Mode.ShaderPipeline);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                    else if (ex is OverflowException)
                    {
                        postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigMaxAttributeSizeByID) +
                          " received a request to convert an invalid string to an int." +
                          " The string must represent a number representable by a signed 32 bit integer.");
                        InvalidateGeneralTextBox(emitter);
                        RemoveJSONDataByID(macroElementParentID, Mode.ShaderPipeline);
                        WriteDXRJSONtoViewGrid();
                        return;
                    }
                }
                // Restore default textbox style.
                Style targetStyle = FindResource("baseTextBoxStyle") as Style;
                if (targetStyle != null)
                {
                    emitter.Style = targetStyle;
                }
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a shader pipeline max attribute size element by ID: " +
                    ex.ToString());
            }
        }
        // Exports
        public void AddRaytracingShaderConfigExportByID(object sender, RoutedEventArgs e)
        {
            try
            {
                TextBox emitter = sender as TextBox;
                if (emitter == null)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigExportByID) +
                        " received a null reference to the TextBox eimitter.");
                    return;
                }
                String macroElementParentID = emitter.Tag as String;
                if (macroElementParentID == null || macroElementParentID.Length == 0)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigExportByID) +
                        " received a zero length ID string.");
                    return;
                }
                String targetText = emitter.Text as String;
                if (targetText == null)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigExportByID) +
                        " received a null content string.");
                    return;
                }
                if (CurrentApplicationMode != Mode.ShaderPipeline)
                {
                    postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigExportByID) +
                        " was initaited from an invalid application mode: " + CurrentApplicationMode);
                    return;
                }
                // Find the JSON element with the correct ID, if it exists.
                // If it doesn't exist, create it.
                DXR_JSON_STATE__RaytracingShaderConfig jsonRaytracingShaderConfig = null;
                foreach (DXR_JSON_STATE__RaytracingShaderConfig shaderConfig in Master_ShaderPipelineConfigByStringID)
                {
                    if (shaderConfig.macroElementParentID == macroElementParentID)
                    {
                        jsonRaytracingShaderConfig = shaderConfig;
                        break;
                    }
                }
                // Create new JSON element if one has not been created.
                if (jsonRaytracingShaderConfig == null)
                {
                    jsonRaytracingShaderConfig = new DXR_JSON_STATE__RaytracingShaderConfig()
                    {
                        macroElementParentID = macroElementParentID
                    };
                    if (jsonRaytracingShaderConfig == null)
                    {
                        postErrorMessage("A call to " + nameof(AddRaytracingShaderConfigExportByID) +
                            " failed to find or create a valid DXR_JSON_STATE__RaytracingShaderConfig");
                        return;
                    }
                    // Add the new JSON element to the master list.
                    Master_ShaderPipelineConfigByStringID.Add(jsonRaytracingShaderConfig);
                }
                jsonRaytracingShaderConfig.Exports = CleanTargetText(targetText);
                // Update JSON View
                WriteDXRJSONtoViewGrid();
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to add a shader pipeline exports element by ID: " +
                    ex.ToString());
            }
        }
        private MacroElement JSONCreateMacroElementByApplicationMode(Mode mode)
        {
            SetApplicationMode(mode);
            MacroElement macroElement = CreateMacroElementByApplicationMode(mode);
            return macroElement;
        }
        private void ConstructMacroElementsFromJSON(List<JsonFileData> JsonFileData)
        {
            try
            {
                if (JsonFileData == null)
                {
                    postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                        " received a null JSON data list.");
                    return;
                }
                MacroElement macroElement;
                foreach (JsonFileData jsonData in JsonFileData)
                {
                    if (jsonData.jsonRoot == null)
                    {
                        postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                        " received a null JsonData.jsonRoot from the JsonFileData list.");
                        continue;
                    }
                    if (jsonData.jsonRoot.DXRState == null)
                    {
                        // This would be a warning, possible but _may_ indicate issues with the underlying file.
                        postWarningMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                        " received a null JsonData.jsonRoot.DXRState from the JsonFileData list.");
                        continue;
                    }
                    if (jsonData.jsonRoot.DXRState.Shaders != null)
                    {
                        IList<DXR_JSON_STATE__Shader> jsonShaderObjList = jsonData.jsonRoot.DXRState.Shaders;
                        if (jsonShaderObjList != null)
                        {
                            uint expectedModeCount = GetModeCount(Mode.Shader);
                            // Create a new MacroElement of a particular mode.
                            macroElement = JSONCreateMacroElementByApplicationMode(Mode.Shader);
                            // Depending on the element type, set the element to reflect the JSON data.
                            foreach (String subElementName in macroElement.regiteredChildrenNames)
                            {
                                // Reset target text
                                String targetText = "";
                                foreach (DXR_JSON_STATE__Shader jsonShaderObj in jsonShaderObjList)
                                {
                                    if (jsonShaderObj == null)
                                    {
                                        continue;
                                    }
                                    // | Shader, Type |
                                    if (subElementName == ("shaderTypeComboBox" + expectedModeCount))
                                    {
                                        if (jsonShaderObj.Type != null &&
                                            jsonShaderObj.Type.Length > 0)
                                        {
                                            ComboBox targetComboBox = FindName(subElementName) as ComboBox;
                                            if (targetComboBox == null)
                                            {
                                                postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    " was unable to acquire the target ComboBox for sub element " + expectedModeCount +
                                                    " for Shader::Type");
                                                return;
                                            }
                                            // Note: Purposeful discrepency between GUI text representation and JSON text representation
                                            if (jsonShaderObj.Type == "Binary")
                                            {
                                                jsonShaderObj.Type = ConstStringBinaryDXILType;
                                            }

                                            if (VerifyComboBoxType(targetComboBox, jsonShaderObj.Type))
                                            {
                                                targetComboBox.Text = jsonShaderObj.Type;
                                                continue;
                                            }
                                            else
                                            {
                                                postWarningMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    "received a request to add an invalid Shader::Type from a JSON source file.");
                                            }
                                        }
                                    }
                                    // | Shader, Entry Point |
                                    if (jsonShaderObj.EntryPoints != null &&
                                        subElementName == ("shaderEntryPointTextBox" + expectedModeCount))
                                    {
                                        foreach (DXR_JSON_STATE__Shader__EntryPoints entryPoint in jsonShaderObj.EntryPoints)
                                        {
                                            // Entry point values need to be parsed from a JSON array into a comma seperated string list.
                                            if (targetText.Length > 0)
                                            {
                                                targetText += ", ";
                                            }
                                            if (entryPoint.ExportName != null &&
                                                entryPoint.ExportName.Length > 0 &&
                                                entryPoint.LinkageName != null &&
                                                entryPoint.LinkageName.Length > 0)
                                            {
                                                targetText += "<" + entryPoint.ExportName + ", " + entryPoint.LinkageName + ">";
                                            }
                                            else if (entryPoint.ExportName != null && entryPoint.ExportName.Length > 0)
                                            {
                                                targetText += entryPoint.ExportName;
                                            }
                                            else
                                            {
                                                postWarningMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    " could not parse the Shader data of the provided JSON.");
                                            }
                                        }
                                    }
                                    // | Shader, File Path |
                                    else if (subElementName == ("shaderFilePathTextBox" + expectedModeCount))
                                    {
                                        if (jsonShaderObj.FilePath != null &&
                                            jsonShaderObj.FilePath.Length > 0)
                                        {
                                            targetText = jsonShaderObj.FilePath;
                                        }
                                    }
                                }
                                // | Shader, Write Data |
                                if (targetText.Length > 0)
                                {
                                    TextBox targetTextBox = FindName(subElementName) as TextBox;
                                    if (targetTextBox != null)
                                    {
                                        targetTextBox.Text = targetText;
                                    }
                                    else
                                    {
                                        postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                            " was unable to find the resouce with name: " +
                                            subElementName);
                                    }
                                }
                                // Change the visibility of macro element separators based on mode count.
                                if (macroElement.hasItemSeperator)
                                {
                                    SetMacroElementSeparatorVisibilityByModeCount();
                                }
                            }
                        }
                    }
                    if (jsonData.jsonRoot.DXRState.HitGroups != null)
                    {
                        IList<DXR_JSON_STATE__HitGroup> jsonHitGroupObjList = jsonData.jsonRoot.DXRState.HitGroups;
                        if (jsonHitGroupObjList != null)
                        {
                            uint expectedModeCount = GetModeCount(Mode.HitGroup);
                            // Create a new MacroElement of a particular mode.
                            macroElement = JSONCreateMacroElementByApplicationMode(Mode.HitGroup);
                            // Depending on the element type, set the element to reflect the JSON data.
                            foreach (String subElementName in macroElement.regiteredChildrenNames)
                            {
                                // Reset target text
                                String targetText = "";
                                foreach (DXR_JSON_STATE__HitGroup jsonHitGroupObj in jsonHitGroupObjList)
                                {
                                    if (jsonHitGroupObj == null)
                                    {
                                        continue;
                                    }
                                    // | HitGroup, Type |
                                    if (subElementName == ("hitGroupTypeComboBox" + expectedModeCount))
                                    {
                                        if (jsonHitGroupObj.Type != null &&
                                            jsonHitGroupObj.Type.Length > 0)
                                        {
                                            ComboBox targetComboBox = FindName(subElementName) as ComboBox;
                                            if (targetComboBox == null)
                                            {
                                                postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    " was unable to acquire the target ComboBox for sub element " + expectedModeCount +
                                                    " for HitGroup::Type");
                                                return;
                                            }
                                            if (VerifyComboBoxType(targetComboBox, jsonHitGroupObj.Type))
                                            {
                                                targetComboBox.Text = jsonHitGroupObj.Type;
                                                continue;
                                            }
                                            else
                                            {
                                                postWarningMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    "received a request to add an invalid HitGroup::Type from a JSON source file.");
                                            }
                                        }
                                    }
                                    // | HitGroup, Name |
                                    else if (subElementName == ("hitGroupNameTextBox" + expectedModeCount))
                                    {
                                        if (jsonHitGroupObj.Name != null &&
                                            jsonHitGroupObj.Name.Length > 0)
                                        {
                                            targetText = jsonHitGroupObj.Name;
                                        }
                                    }
                                    // | HitGroup, IntersectionShader |
                                    else if (subElementName == ("hitGroupIntersectionShaderTextBox" + expectedModeCount))
                                    {
                                        if (jsonHitGroupObj.IntersectionShader != null &&
                                            jsonHitGroupObj.IntersectionShader.Length > 0)
                                        {
                                            targetText = jsonHitGroupObj.IntersectionShader;
                                        }
                                    }
                                    // | HitGroup, AnyHitShader |
                                    else if (subElementName == ("hitGroupAnyHitShaderTextBox" + expectedModeCount))
                                    {
                                        if (jsonHitGroupObj.AnyHitShader != null &&
                                            jsonHitGroupObj.AnyHitShader.Length > 0)
                                        {
                                            targetText = jsonHitGroupObj.AnyHitShader;
                                        }
                                    }
                                    // | HitGroup, ClosestHitShader |
                                    else if (subElementName == ("hitGroupClosestHitShaderTextBox" + expectedModeCount))
                                    {
                                        if (jsonHitGroupObj.ClosestHitShader != null &&
                                            jsonHitGroupObj.ClosestHitShader.Length > 0)
                                        {
                                            targetText = jsonHitGroupObj.ClosestHitShader;
                                        }
                                    }
                                }
                                // | HitGroup, Write Data |
                                if (targetText.Length > 0)
                                {
                                    TextBox targetTextBox = FindName(subElementName) as TextBox;
                                    if (targetTextBox != null)
                                    {
                                        targetTextBox.Text = targetText;
                                    }
                                    else
                                    {
                                        postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                            " was unable to find the resouce with name: " +
                                            subElementName);
                                    }
                                }
                                // Change the visibility of macro element separators based on mode count.
                                if (macroElement.hasItemSeperator)
                                {
                                    SetMacroElementSeparatorVisibilityByModeCount();
                                }
                            }
                        }
                    }
                    if (jsonData.jsonRoot.DXRState.LocalRootSignatures != null)
                    {
                        IList<DXR_JSON_STATE__RootSignature> jsonLocalRootSignaturesObjList =
                            jsonData.jsonRoot.DXRState.LocalRootSignatures;
                        if (jsonLocalRootSignaturesObjList != null)
                        {
                            uint expectedModeCount = GetModeCount(Mode.LocalRootSignature);
                            // Create a new MacroElement of a particular mode.
                            macroElement = JSONCreateMacroElementByApplicationMode(Mode.LocalRootSignature);
                            // Depending on the element type, set the element to reflect the JSON data.
                            foreach (String subElementName in macroElement.regiteredChildrenNames)
                            {
                                // Reset target text
                                String targetText = "";
                                foreach (DXR_JSON_STATE__RootSignature jsonLocalRootSignaturesObj in jsonLocalRootSignaturesObjList)
                                {
                                    if (jsonLocalRootSignaturesObj == null)
                                    {
                                        continue;
                                    }
                                    // | LocalRootSignatures, Type |
                                    if (subElementName == ("localRootSignaturesTypeComboBox" + expectedModeCount))
                                    {
                                        if (jsonLocalRootSignaturesObj.Type != null &&
                                            jsonLocalRootSignaturesObj.Type.Length > 0)
                                        {
                                            ComboBox targetComboBox = FindName(subElementName) as ComboBox;
                                            if (targetComboBox == null)
                                            {
                                                postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    " was unable to acquire the target ComboBox for sub element " + expectedModeCount +
                                                    " for LocalRootSignature::Type");
                                                return;
                                            }
                                            // Note: Purposeful discrepency between GUI text representation and JSON text representation
                                            if (jsonLocalRootSignaturesObj.Type == "Binary")
                                            {
                                                jsonLocalRootSignaturesObj.Type = ConstStringBinaryDXILType;
                                            }

                                            if (VerifyComboBoxType(targetComboBox, jsonLocalRootSignaturesObj.Type))
                                            {
                                                targetComboBox.Text = jsonLocalRootSignaturesObj.Type;
                                                continue;
                                            }
                                            else
                                            {
                                                postWarningMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    "received a request to add an invalid LocalRootSignature::Type from a JSON source file.");
                                            }
                                        }
                                    }
                                    // | LocalRootSignatures, FilePath |
                                    if (subElementName == ("localRootSignaturesFilePathTextBox" + expectedModeCount))
                                    {
                                        if (jsonLocalRootSignaturesObj.FilePath != null &&
                                            jsonLocalRootSignaturesObj.FilePath.Length > 0)
                                        {
                                            targetText = jsonLocalRootSignaturesObj.FilePath;
                                        }
                                    }
                                    // | LocalRootSignatures, Name |
                                    else if (subElementName == ("localRootSignaturesMacroNameShaderTextBox" + expectedModeCount))
                                    {
                                        if (jsonLocalRootSignaturesObj.Name != null &&
                                            jsonLocalRootSignaturesObj.Name.Length > 0)
                                        {
                                            targetText = jsonLocalRootSignaturesObj.Name;
                                        }
                                    }
                                    // | LocalRootSignatures, Exports |
                                    else if (subElementName == ("localRootSignaturesExportsTextBox" + expectedModeCount))
                                    {
                                        if (jsonLocalRootSignaturesObj.Exports == null)
                                        {
                                            continue;
                                        }
                                        foreach (String rootSignature in jsonLocalRootSignaturesObj.Exports)
                                        {
                                            if (jsonLocalRootSignaturesObj.Exports == null)
                                            {
                                                jsonLocalRootSignaturesObj.Exports = new List<String>();
                                            }
                                            if (rootSignature != null)
                                            {
                                                // Exports values need to be parsed from a JSON array into a comma seperated string list.
                                                if (targetText.Length > 0)
                                                {
                                                    targetText += ", ";
                                                }
                                                targetText += rootSignature;
                                            }
                                        }
                                    }
                                }
                                // | LocalRootSignatures, Write Data |
                                if (targetText.Length > 0)
                                {
                                    TextBox targetTextBox = FindName(subElementName) as TextBox;
                                    if (targetTextBox != null)
                                    {
                                        targetTextBox.Text = targetText;
                                    }
                                    else
                                    {
                                        postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                            " was unable to find the resouce with name: " +
                                            subElementName);
                                    }
                                }
                                // Change the visibility of macro element separators based on mode count.
                                if (macroElement.hasItemSeperator)
                                {
                                    SetMacroElementSeparatorVisibilityByModeCount();
                                }
                            }
                        }
                    }
                    if (jsonData.jsonRoot.DXRState.GlobalRootSignatures != null)
                    {
                        IList<DXR_JSON_STATE__RootSignature> jsonGlobalRootSignaturesObjList =
                            jsonData.jsonRoot.DXRState.GlobalRootSignatures;
                        if (jsonGlobalRootSignaturesObjList != null)
                        {
                            uint expectedModeCount = GetModeCount(Mode.GlobalRootSignature);
                            // Create a new MacroElement of a particular mode.
                            macroElement = JSONCreateMacroElementByApplicationMode(Mode.GlobalRootSignature);
                            // Depending on the element type, set the element to reflect the JSON data.
                            foreach (String subElementName in macroElement.regiteredChildrenNames)
                            {
                                if (subElementName == null)
                                {
                                    continue;
                                }
                                // Reset target text
                                String targetText = "";
                                foreach (DXR_JSON_STATE__RootSignature jsonGlobalRootSignaturesObj in jsonGlobalRootSignaturesObjList)
                                {
                                    if (jsonGlobalRootSignaturesObj == null)
                                    {
                                        continue;
                                    }
                                    // | GlobalRootSignatures, Type |
                                    if (subElementName == ("globalRootSignaturesTypeComboBox" + expectedModeCount))
                                    {
                                        if (jsonGlobalRootSignaturesObj.Type != null &&
                                            jsonGlobalRootSignaturesObj.Type.Length > 0)
                                        {
                                            ComboBox targetComboBox = FindName(subElementName) as ComboBox;
                                            if (targetComboBox == null)
                                            {
                                                postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    " was unable to acquire the target ComboBox for sub element " + expectedModeCount +
                                                    " for GlobalRootSignature::Type");
                                                return;
                                            }
                                            // Note: Purposeful discrepency between GUI text representation and JSON text representation
                                            if (jsonGlobalRootSignaturesObj.Type == "Binary")
                                            {
                                                jsonGlobalRootSignaturesObj.Type = ConstStringBinaryDXILType;
                                            }

                                            if (VerifyComboBoxType(targetComboBox, jsonGlobalRootSignaturesObj.Type))
                                            {
                                                targetComboBox.Text = jsonGlobalRootSignaturesObj.Type;
                                                continue;
                                            }
                                            else
                                            {
                                                postWarningMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                    "received a request to add an invalid GlobalRootSignature::Type from a JSON source file.");
                                            }
                                        }
                                    }
                                    // | GlobalRootSignatures, FilePath |
                                    if (subElementName == ("globalRootSignaturesFilePathTextBox" + expectedModeCount))
                                    {
                                        if (jsonGlobalRootSignaturesObj.FilePath != null &&
                                            jsonGlobalRootSignaturesObj.FilePath.Length > 0)
                                        {
                                            targetText = jsonGlobalRootSignaturesObj.FilePath;
                                        }
                                    }
                                    // | GlobalRootSignatures, Name |
                                    else if (subElementName == ("globalRootSignaturesMacroNameShaderTextBox" + expectedModeCount))
                                    {
                                        if (jsonGlobalRootSignaturesObj.Name != null &&
                                            jsonGlobalRootSignaturesObj.Name.Length > 0)
                                        {
                                            targetText = jsonGlobalRootSignaturesObj.Name;
                                        }
                                    }
                                    // | GlobalRootSignatures, Exports |
                                    else if (subElementName == ("globalRootSignaturesExportsTextBox" + expectedModeCount))
                                    {
                                        if (jsonGlobalRootSignaturesObj.Exports == null)
                                        {
                                            jsonGlobalRootSignaturesObj.Exports = new List<String>();
                                        }
                                        foreach (String rootSignature in jsonGlobalRootSignaturesObj.Exports)
                                        {
                                            if (rootSignature != null)
                                            {
                                                // Exports values need to be parsed from a JSON array into a comma seperated string list.
                                                if (targetText.Length > 0)
                                                {
                                                    targetText += ", ";
                                                }
                                                targetText += rootSignature;
                                            }
                                        }
                                    }
                                }
                                // | GlobalRootSignatures, Write Data |
                                if (targetText.Length > 0)
                                {
                                    TextBox targetTextBox = FindName(subElementName) as TextBox;
                                    if (targetTextBox != null)
                                    {
                                        targetTextBox.Text = targetText;
                                    }
                                    else
                                    {
                                        postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                            " was unable to find the resouce with name: " +
                                            subElementName);
                                    }
                                }
                                // Change the visibility of macro element separators based on mode count.
                                if (macroElement.hasItemSeperator)
                                {
                                    SetMacroElementSeparatorVisibilityByModeCount();
                                }
                            }
                        }
                    }
                    if (jsonData.jsonRoot.DXRState.RaytracingPipelineConfig != null)
                    {
                        SetApplicationMode(Mode.RaytracingPipeline);
                        uint elementsFound = 0;
                        foreach (MacroElement MacroElement in Master_MacroElementList)
                        {
                            if (MacroElement.applicationMode == Mode.RaytracingPipeline)
                            {
                                elementsFound++;
                            }
                        }
                        if (elementsFound > 1)
                        {
                            postErrorMessage("Found more than one instance of the RaytracingPipelienConfig MacroElement.");
                        }
                        // | RaytracingPipelineConfig, Max Trace Recursion Depth |
                        String subElementName = "RaytracingPipelineMaxTraceRecursionDepthTextBox0";
                        TextBox targetTextBox = FindName(subElementName) as TextBox;
                        if (targetTextBox != null)
                        {
                            targetTextBox.Text =
                                jsonData.jsonRoot.DXRState.RaytracingPipelineConfig.MaxTraceRecursionDepth.ToString();
                        }
                        else
                        {
                            postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                " was unable to find the resouce with name: " +
                                subElementName);
                        }
                        // | RaytracingPipelineConfig, Exports |
                        subElementName = "RaytracingPipelineExportsTextBox0";
                        targetTextBox = FindName(subElementName) as TextBox;
                        if (targetTextBox != null)
                        {
                            String targetText = "";
                            if (jsonData.jsonRoot.DXRState.RaytracingPipelineConfig.Exports != null)
                            {
                                foreach (String export in jsonData.jsonRoot.DXRState.RaytracingPipelineConfig.Exports)
                                {
                                    if (export != null)
                                    {
                                        // Exports values need to be parsed from a JSON array into a comma seperated string list.
                                        if (targetText.Length > 0)
                                        {
                                            targetText += ", ";
                                        }
                                        targetText += export;
                                    }
                                }
                                targetTextBox.Text = targetText;
                            }
                        }
                        else
                        {
                            postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                " was unable to find the resouce with name: " +
                                subElementName);
                        }
                    }
                    if (jsonData.jsonRoot.DXRState.RaytracingShaderConfig != null)
                    {
                        IList<DXR_JSON_STATE__RaytracingShaderConfig> jsonRaytracingShaderConfigObjList =
                            jsonData.jsonRoot.DXRState.RaytracingShaderConfig;
                        if (jsonRaytracingShaderConfigObjList != null)
                        {
                            uint expectedModeCount = GetModeCount(Mode.ShaderPipeline);
                            // Create a new MacroElement of a particular mode.
                            macroElement = JSONCreateMacroElementByApplicationMode(Mode.ShaderPipeline);
                            // Depending on the element type, set the element to reflect the JSON data.
                            foreach (String subElementName in macroElement.regiteredChildrenNames)
                            {
                                if (subElementName == null)
                                {
                                    continue;
                                }
                                if (subElementName != null)
                                {
                                    // Reset target text
                                    String targetText = "";
                                    foreach (DXR_JSON_STATE__RaytracingShaderConfig jsonRaytracingShaderConfigObj
                                        in jsonRaytracingShaderConfigObjList)
                                    {
                                        if (jsonRaytracingShaderConfigObj == null)
                                        {
                                            continue;
                                        }
                                        // | RaytracingShaderConfig, PayloadSizeInBytes |
                                        if (subElementName == ("ShaderPipelinePayloadSizeTextBox" + expectedModeCount))
                                        {
                                            targetText = jsonRaytracingShaderConfigObj.MaxPayloadSizeInBytes.ToString();
                                        }
                                        // | RaytracingShaderConfig, MaxAttributeSizeInBytes |
                                        else if (subElementName == ("ShaderPipelinesMaxAttributeSizeTextBox" + expectedModeCount))
                                        {
                                            targetText = jsonRaytracingShaderConfigObj.MaxAttributeSizeInBytes.ToString();
                                        }
                                        // | RaytracingShaderConfig, Exports |
                                        else if (subElementName == ("ShaderPipelinesExportsTextBox" + expectedModeCount))
                                        {
                                            if (jsonRaytracingShaderConfigObj.Exports == null)
                                            {
                                                jsonRaytracingShaderConfigObj.Exports = new List<String>();
                                            }
                                            foreach (String rtShaderConfigObj in jsonRaytracingShaderConfigObj.Exports)
                                            {
                                                if (rtShaderConfigObj != null)
                                                {
                                                    // Exports values need to be parsed from a JSON array into a comma seperated string list.
                                                    if (targetText.Length > 0)
                                                    {
                                                        targetText += ", ";
                                                    }
                                                    targetText += rtShaderConfigObj;
                                                }
                                            }
                                        }
                                    }
                                    // | RaytracingShaderConfig, Write Data |
                                    if (targetText.Length > 0)
                                    {
                                        TextBox targetTextBox = FindName(subElementName) as TextBox;
                                        if (targetTextBox != null)
                                        {
                                            targetTextBox.Text = targetText;
                                        }
                                        else
                                        {
                                            postErrorMessage("A call to " + nameof(ConstructMacroElementsFromJSON) +
                                                " was unable to find the resouce with name: " +
                                                subElementName);
                                        }
                                    }
                                    // Change the visibility of macro element separators based on mode count.
                                    if (macroElement.hasItemSeperator)
                                    {
                                        SetMacroElementSeparatorVisibilityByModeCount();
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to construct macro elements from JSON data: " +
                    ex.ToString());
            }
        }
        private void ReadJSONFileContents(String[] files, List<JsonFileData> jsonFileDataList)
        {
            foreach (String file in files)
            {
                if (file != null)
                {
                    if (System.IO.File.Exists(file))
                    {
                        String tmpString = System.IO.File.ReadAllText(file);
                        if (tmpString.Length > 0)
                        {
                            JsonFileData data = new JsonFileData();
                            data.filePath = file;
                            data.fileContents = tmpString;
                            try
                            {
                                data.jsonRoot = JsonSerializer.Deserialize<DXR_JSON_STATE_ROOT>(tmpString, jsonOptions);
                            }
                            catch (JsonException jex)
                            {
                                String errMsg = "";
                                errMsg += "An error occured while parsing the follwowing JSON file:\n\n" + file + "\n\n";
                                errMsg += "Offending line number: " + jex.LineNumber + "\n";
                                errMsg += "Offending line byte offset: " + jex.BytePositionInLine + "\n\n";
                                errMsg += "\nThis file's contents will not be added.";
                                MessageBoxResult msgBox = MessageBox.Show(errMsg,
                                    "JSON Parsing Error",
                                    MessageBoxButton.OK,
                                    MessageBoxImage.Error);
                            }
                            jsonFileDataList.Add(data);
                        }
                    }
                    else
                    {
                        postErrorMessage("A call to " + nameof(ReadJSONFileContents) +
                            " received a file handle which does not correspond to an existing file.");
                        return;
                    }
                }
            }
        }
#if false
        // Reintroduce if drag and drop functionality is reinstated.
        private void HandleJSONDragAndDrop(object sender, DragEventArgs e)
        {
            try
            {
                List<JsonFileData> jsonFileDataList = new List<JsonFileData>();
                String[] files = (String[])e.Data.GetData(DataFormats.FileDrop);
                if (files != null && files.Length > 0)
                {
                    ReadJSONFileContents(files, jsonFileDataList);
                    if (jsonFileDataList.Count > 0)
                    {
                        ConstructMacroElementsFromJSON(jsonFileDataList);
                    }
                }
            }
            catch (System.Exception ex)
            {
                postErrorMessage("An unhandled exception has been caught while attempting to handle JSON file drag-and-drop: " +
                    ex.ToString());
            }
        }
#endif
    }
}