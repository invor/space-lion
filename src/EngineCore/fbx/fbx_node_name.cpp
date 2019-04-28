
#include "fbx_node_name.hpp"

#include <unordered_map>

namespace FBX {
	const static std::unordered_map<std::string, NodeName::NodeNameE> node_names_strToEnum {
		{ "Actor"                        , NodeName::Actor                        },
		{ "Add"                          , NodeName::Add                          },
		{ "Additive"                     , NodeName::Additive                     },
		{ "Aim"                          , NodeName::Aim                          },
		{ "AllSame"                      , NodeName::AllSame                      },
		{ "Alpha"                        , NodeName::Alpha                        },
		{ "Alphas"                       , NodeName::Alphas                       },
		{ "Ambient"                      , NodeName::Ambient                      },
		{ "AmbientLightColor"            , NodeName::AmbientLightColor            },
		{ "AmbientRenderSettings"        , NodeName::AmbientRenderSettings        },
		{ "AnimationCurve"               , NodeName::AnimationCurve               },
		{ "AnimationCurveNode"           , NodeName::AnimationCurveNode           },
		{ "AnimationEvaluator"           , NodeName::AnimationEvaluator           },
		{ "AnimationLayer"               , NodeName::AnimationLayer               },
		{ "AnimationMode"                , NodeName::AnimationMode                },
		{ "AnimationStack"               , NodeName::AnimationStack               },
		{ "Antialiasing"                 , NodeName::Antialiasing                 },
		{ "Aperture"                     , NodeName::Aperture                     },
		{ "ApertureFormat"               , NodeName::ApertureFormat               },
		{ "ApertureMode"                 , NodeName::ApertureMode                 },
		{ "AspectH"                      , NodeName::AspectH                      },
		{ "AspectType"                   , NodeName::AspectType                   },
		{ "AspectW"                      , NodeName::AspectW                      },
		{ "AssociateModel"               , NodeName::AssociateModel               },
		{ "AudioColor"                   , NodeName::AudioColor                   },
		{ "Author"                       , NodeName::Author                       },
		{ "Average"                      , NodeName::Average                      },
		{ "AxisLen"                      , NodeName::AxisLen                      },
		{ "BackGroundColor"              , NodeName::BackGroundColor              },
		{ "BackgroundMode"               , NodeName::BackgroundMode               },
		{ "BackgroundTexture"            , NodeName::BackgroundTexture            },
		{ "BackgroundTreshold"           , NodeName::BackgroundTreshold           },
		{ "BindingOperator"              , NodeName::BindingOperator              },
		{ "BindingTable"                 , NodeName::BindingTable                 },
		{ "BindPose"                     , NodeName::BindPose                     },
		{ "Binormals"                    , NodeName::Binormals                    },
		{ "BinormalsIndex"               , NodeName::BinormalsIndex               },
		{ "BinormalsW"                   , NodeName::BinormalsW                   },
		{ "Blend"                        , NodeName::Blend                        },
		{ "BlendMode"                    , NodeName::BlendMode                    },
		{ "BlendModes"                   , NodeName::BlendModes                   },
		{ "BlendShape"                   , NodeName::BlendShape                   },
		{ "BlendShapeChannel"            , NodeName::BlendShapeChannel            },
		{ "BlendWeights"                 , NodeName::BlendWeights                 },
		{ "Boundary"                     , NodeName::Boundary                     },
		{ "BoundaryEdge"                 , NodeName::BoundaryEdge                 },
		{ "BoundaryRule"                 , NodeName::BoundaryRule                 },
		{ "BoundaryVersion"              , NodeName::BoundaryVersion              },
		{ "BumpNormalMap"                , NodeName::BumpNormalMap                },
		{ "ByEdge"                       , NodeName::ByEdge                       },
		{ "ByFace"                       , NodeName::ByFace                       },
		{ "ByModel"                      , NodeName::ByModel                      },
		{ "ByPolygon"                    , NodeName::ByPolygon                    },
		{ "ByPolygonVertex"              , NodeName::ByPolygonVertex              },
		{ "ByVertice"                    , NodeName::ByVertice                    },
		{ "C"                            , NodeName::C                            },
		{ "Cache"                        , NodeName::Cache                        },
		{ "CacheChannel"                 , NodeName::CacheChannel                 },
		{ "CachePath"                    , NodeName::CachePath                    },
		{ "Camera"                       , NodeName::Camera                       },
		{ "CameraAndLens"                , NodeName::CameraAndLens                },
		{ "CameraAperture"               , NodeName::CameraAperture               },
		{ "CameraId"                     , NodeName::CameraId                     },
		{ "CameraIndexName"              , NodeName::CameraIndexName              },
		{ "CameraLock"                   , NodeName::CameraLock                   },
		{ "CameraName"                   , NodeName::CameraName                   },
		{ "CameraOrthoZoom"              , NodeName::CameraOrthoZoom              },
		{ "CastLight"                    , NodeName::CastLight                    },
		{ "Character"                    , NodeName::Character                    },
		{ "Children"                     , NodeName::Children                     },
		{ "Clip"                         , NodeName::Clip                         },
		{ "Closed"                       , NodeName::Closed                       },
		{ "Collection"                   , NodeName::Collection                   },
		{ "CollectionExclusive"          , NodeName::CollectionExclusive          },
		{ "Color"                        , NodeName::Color                        },
		{ "Colorburn"                    , NodeName::Colorburn                    },
		{ "Colordodge"                   , NodeName::Colordodge                   },
		{ "ColorIndex"                   , NodeName::ColorIndex                   },
		{ "Colors"                       , NodeName::Colors                       },
		{ "Comment"                      , NodeName::Comment                      },
		{ "Comments"                     , NodeName::Comments                     },
		{ "Component"                    , NodeName::Component                    },
		{ "Connections"                  , NodeName::Connections                  },
		{ "Constraint"                   , NodeName::Constraint                   },
		{ "Consumer"                     , NodeName::Consumer                     },
		{ "Consumers"                    , NodeName::Consumers                    },
		{ "Container"                    , NodeName::Container                    },
		{ "Content"                      , NodeName::Content                      },
		{ "ContentCount"                 , NodeName::ContentCount                 },
		{ "ControlSetPlug"               , NodeName::ControlSetPlug               },
		{ "Count"                        , NodeName::Count                        },
		{ "CreationTime"                 , NodeName::CreationTime                 },
		{ "CreationTimeStamp"            , NodeName::CreationTimeStamp            },
		{ "Creator"                      , NodeName::Creator                      },
		{ "Cropping"                     , NodeName::Cropping                     },
		{ "Culling"                      , NodeName::Culling                      },
		{ "CullingOff"                   , NodeName::CullingOff                   },
		{ "CullingOnCCW"                 , NodeName::CullingOnCCW                 },
		{ "CullingOnCW"                  , NodeName::CullingOnCW                  },
		{ "Current"                      , NodeName::Current                      },
		{ "CurrentLevel"                 , NodeName::CurrentLevel                 },
		{ "Custom"                       , NodeName::Custom                       },
		{ "Darken"                       , NodeName::Darken                       },
		{ "Darkercolor"                  , NodeName::Darkercolor                  },
		{ "Day"                          , NodeName::Day                          },
		{ "Default"                      , NodeName::Default                      },
		{ "DefaultCamera"                , NodeName::DefaultCamera                },
		{ "DefaultViewingMode"           , NodeName::DefaultViewingMode           },
		{ "Definitions"                  , NodeName::Definitions                  },
		{ "Deformer"                     , NodeName::Deformer                     },
		{ "DeformPercent"                , NodeName::DeformPercent                },
		{ "DepthOfField"                 , NodeName::DepthOfField                 },
		{ "DestinationCount"             , NodeName::DestinationCount             },
		{ "Device"                       , NodeName::Device                       },
		{ "Difference"                   , NodeName::Difference                   },
		{ "Diffuse"                      , NodeName::Diffuse                      },
		{ "Dimension"                    , NodeName::Dimension                    },
		{ "Dimensions"                   , NodeName::Dimensions                   },
		{ "Direct"                       , NodeName::Direct                       },
		{ "DisplayLayer"                 , NodeName::DisplayLayer                 },
		{ "DisplayMode"                  , NodeName::DisplayMode                  },
		{ "DisplaySafeArea"              , NodeName::DisplaySafeArea              },
		{ "DisplaySafeAreaOnRender"      , NodeName::DisplaySafeAreaOnRender      },
		{ "DisplaySubdivisions"          , NodeName::DisplaySubdivisions          },
		{ "Dissolve"                     , NodeName::Dissolve                     },
		{ "Divide"                       , NodeName::Divide                       },
		{ "Doc"                          , NodeName::Doc                          },
		{ "Document"                     , NodeName::Document                     },
		{ "Documents"                    , NodeName::Documents                    },
		{ "DrawComponent"                , NodeName::DrawComponent                },
		{ "DualQuaternion"               , NodeName::DualQuaternion               },
		{ "Dynavision"                   , NodeName::Dynavision                   },
		{ "EdgeCrease"                   , NodeName::EdgeCrease                   },
		{ "EdgeIndexArray"               , NodeName::EdgeIndexArray               },
		{ "Edges"                        , NodeName::Edges                        },
		{ "Embedding"                    , NodeName::Embedding                    },
		{ "Emissive"                     , NodeName::Emissive                     },
		{ "EncryptionType"               , NodeName::EncryptionType               },
		{ "Entry"                        , NodeName::Entry                        },
		{ "Exclusion"                    , NodeName::Exclusion                    },
		{ "FarPlane"                     , NodeName::FarPlane                     },
		{ "FBXHeaderExtension"           , NodeName::FBXHeaderExtension           },
		{ "FBXHeaderVersion"             , NodeName::FBXHeaderVersion             },
		{ "FBXVersion"                   , NodeName::FBXVersion                   },
		{ "FieldOfViewXProperty"         , NodeName::FieldOfViewXProperty         },
		{ "FieldOfViewYProperty"         , NodeName::FieldOfViewYProperty         },
		{ "File"                         , NodeName::File                         },
		{ "FileId"                       , NodeName::FileId                       },
		{ "Filename"                     , NodeName::Filename                     },
		{ "FileName"                     , NodeName::FileName                     },
		{ "FilePathUrl"                  , NodeName::FilePathUrl                  },
		{ "FilmOffsetLeftCam"            , NodeName::FilmOffsetLeftCam            },
		{ "FilmOffsetRightCam"           , NodeName::FilmOffsetRightCam           },
		{ "FlipNormals"                  , NodeName::FlipNormals                  },
		{ "FlogEnable"                   , NodeName::FlogEnable                   },
		{ "FocalLength"                  , NodeName::FocalLength                  },
		{ "FogColor"                     , NodeName::FogColor                     },
		{ "FogDensity"                   , NodeName::FogDensity                   },
		{ "FogEnd"                       , NodeName::FogEnd                       },
		{ "FogMode"                      , NodeName::FogMode                      },
		{ "FogOptions"                   , NodeName::FogOptions                   },
		{ "FogStart"                     , NodeName::FogStart                     },
		{ "Folder"                       , NodeName::Folder                       },
		{ "ForegroundTransparent"        , NodeName::ForegroundTransparent        },
		{ "Form"                         , NodeName::Form                         },
		{ "Format"                       , NodeName::Format                       },
		{ "FormatName"                   , NodeName::FormatName                   },
		{ "FrameColor"                   , NodeName::FrameColor                   },
		{ "FrameRate"                    , NodeName::FrameRate                    },
		{ "FullWeights"                  , NodeName::FullWeights                  },
		{ "GenericNode"                  , NodeName::GenericNode                  },
		{ "Geometry"                     , NodeName::Geometry                     },
		{ "GeometryLayer"                , NodeName::GeometryLayer                },
		{ "GeometryUVInfo"               , NodeName::GeometryUVInfo               },
		{ "GeometryVersion"              , NodeName::GeometryVersion              },
		{ "GeometryWeightedMap"          , NodeName::GeometryWeightedMap          },
		{ "GlobalSettings"               , NodeName::GlobalSettings               },
		{ "Gobo"                         , NodeName::Gobo                         },
		{ "GoboManager"                  , NodeName::GoboManager                  },
		{ "GoboName"                     , NodeName::GoboName                     },
		{ "GoboPath"                     , NodeName::GoboPath                     },
		{ "Hardmix"                      , NodeName::Hardmix                      },
		{ "HD"                           , NodeName::HD                           },
		{ "Hidden"                       , NodeName::Hidden                       },
		{ "HierarchyView"                , NodeName::HierarchyView                },
		{ "Hole"                         , NodeName::Hole                         },
		{ "Hour"                         , NodeName::Hour                         },
		{ "Hue"                          , NodeName::Hue                          },
		{ "IKPivot"                      , NodeName::IKPivot                      },
		{ "IKPull"                       , NodeName::IKPull                       },
		{ "IKPullHips"                   , NodeName::IKPullHips                   },
		{ "IKReachRotation"              , NodeName::IKReachRotation              },
		{ "IKReachTranslation"           , NodeName::IKReachTranslation           },
		{ "ImageData"                    , NodeName::ImageData                    },
		{ "ImageEncoding"                , NodeName::ImageEncoding                },
		{ "Imax"                         , NodeName::Imax                         },
		{ "Implementation"               , NodeName::Implementation               },
		{ "Index"                        , NodeName::Index                        },
		{ "Indexes"                      , NodeName::Indexes                      },
		{ "IndexMapping"                 , NodeName::IndexMapping                 },
		{ "IndexToDirect"                , NodeName::IndexToDirect                },
		{ "InteraxialSeparation"         , NodeName::InteraxialSeparation         },
		{ "InternalEdges"                , NodeName::InternalEdges                },
		{ "Keywords"                     , NodeName::Keywords                     },
		{ "KFbxEnvironment"              , NodeName::KFbxEnvironment              },
		{ "KnotVector"                   , NodeName::KnotVector                   },
		{ "KnotVectorU"                  , NodeName::KnotVectorU                  },
		{ "KnotVectorV"                  , NodeName::KnotVectorV                  },
		{ "Label"                        , NodeName::Label                        },
		{ "Layer"                        , NodeName::Layer                        },
		{ "LayeredTexture"               , NodeName::LayeredTexture               },
		{ "LayerElement"                 , NodeName::LayerElement                 },
		{ "LayerElementBinormal"         , NodeName::LayerElementBinormal         },
		{ "LayerElementColor"            , NodeName::LayerElementColor            },
		{ "LayerElementEdgeCrease"       , NodeName::LayerElementEdgeCrease       },
		{ "LayerElementHole"             , NodeName::LayerElementHole             },
		{ "LayerElementMaterial"         , NodeName::LayerElementMaterial         },
		{ "LayerElementNormal"           , NodeName::LayerElementNormal           },
		{ "LayerElementPolygonGroup"     , NodeName::LayerElementPolygonGroup     },
		{ "LayerElementSmoothing"        , NodeName::LayerElementSmoothing        },
		{ "LayerElementTangent"          , NodeName::LayerElementTangent          },
		{ "LayerElementTexture"          , NodeName::LayerElementTexture          },
		{ "LayerElementUserData"         , NodeName::LayerElementUserData         },
		{ "LayerElementUV"               , NodeName::LayerElementUV               },
		{ "LayerElementVertexCrease"     , NodeName::LayerElementVertexCrease     },
		{ "LayerElementVisibility"       , NodeName::LayerElementVisibility       },
		{ "LayerTextureInfo"             , NodeName::LayerTextureInfo             },
		{ "LevelCount"                   , NodeName::LevelCount                   },
		{ "Library"                      , NodeName::Library                      },
		{ "Lighten"                      , NodeName::Lighten                      },
		{ "Lightercolor"                 , NodeName::Lightercolor                 },
		{ "LightGobo"                    , NodeName::LightGobo                    },
		{ "LightMap"                     , NodeName::LightMap                     },
		{ "LightType"                    , NodeName::LightType                    },
		{ "LightTypeVersion"             , NodeName::LightTypeVersion             },
		{ "LimbLength"                   , NodeName::LimbLength                   },
		{ "Limits"                       , NodeName::Limits                       },
		{ "Line"                         , NodeName::Line                         },
		{ "Linear"                       , NodeName::Linear                       },
		{ "Linearburn"                   , NodeName::Linearburn                   },
		{ "Lineardodge"                  , NodeName::Lineardodge                  },
		{ "Linearlight"                  , NodeName::Linearlight                  },
		{ "LineVersion"                  , NodeName::LineVersion                  },
		{ "Link"                         , NodeName::Link                         },
		{ "Link_DeformAcuracy"           , NodeName::Link_DeformAcuracy           },
		{ "LocalTime"                    , NodeName::LocalTime                    },
		{ "Look"                         , NodeName::Look                         },
		{ "LookAt"                       , NodeName::LookAt                       },
		{ "LookAtModel"                  , NodeName::LookAtModel                  },
		{ "Loop"                         , NodeName::Loop                         },
		{ "Luminosity"                   , NodeName::Luminosity                   },
		{ "MappingInformationType"       , NodeName::MappingInformationType       },
		{ "MarkerSet"                    , NodeName::MarkerSet                    },
		{ "Material"                     , NodeName::Material                     },
		{ "MaterialAssignation"          , NodeName::MaterialAssignation          },
		{ "MaterialMode"                 , NodeName::MaterialMode                 },
		{ "Materials"                    , NodeName::Materials                    },
		{ "Max"                          , NodeName::Max                          },
		{ "MaxBlend"                     , NodeName::MaxBlend                     },
		{ "Media"                        , NodeName::Media                        },
		{ "Mesh"                         , NodeName::Mesh                         },
		{ "MetaData"                     , NodeName::MetaData                     },
		{ "Millisecond"                  , NodeName::Millisecond                  },
		{ "Min"                          , NodeName::Min                          },
		{ "Minute"                       , NodeName::Minute                       },
		{ "Mode"                         , NodeName::Mode                         },
		{ "Model"                        , NodeName::Model                        },
		{ "ModelUVScaling"               , NodeName::ModelUVScaling               },
		{ "ModelUVTranslation"           , NodeName::ModelUVTranslation           },
		{ "Modulate"                     , NodeName::Modulate                     },
		{ "Modulate2"                    , NodeName::Modulate2                    },
		{ "Month"                        , NodeName::Month                        },
		{ "MultiLayer"                   , NodeName::MultiLayer                   },
		{ "MultiplicityU"                , NodeName::MultiplicityU                },
		{ "MultiplicityV"                , NodeName::MultiplicityV                },
		{ "Name"                         , NodeName::Name                         },
		{ "NearPlane"                    , NodeName::NearPlane                    },
		{ "NodeAttribute"                , NodeName::NodeAttribute                },
		{ "NodeAttributeName"            , NodeName::NodeAttributeName            },
		{ "NodeAttributeRefTo"           , NodeName::NodeAttributeRefTo           },
		{ "NodeId"                       , NodeName::NodeId                       },
		{ "NoMappingInformation"         , NodeName::NoMappingInformation         },
		{ "Normal"                       , NodeName::Normal                       },
		{ "Normals"                      , NodeName::Normals                      },
		{ "NormalsIndex"                 , NodeName::NormalsIndex                 },
		{ "NormalsW"                     , NodeName::NormalsW                     },
		{ "NTSC"                         , NodeName::NTSC                         },
		{ "Nurb"                         , NodeName::Nurb                         },
		{ "NurbOrder"                    , NodeName::NurbOrder                    },
		{ "NurbsCurve"                   , NodeName::NurbsCurve                   },
		{ "NurbsCurveVersion"            , NodeName::NurbsCurveVersion            },
		{ "NurbsSurface"                 , NodeName::NurbsSurface                 },
		{ "NurbsSurfaceOrder"            , NodeName::NurbsSurfaceOrder            },
		{ "NurbsSurfaceVersion"          , NodeName::NurbsSurfaceVersion          },
		{ "NurbSurface"                  , NodeName::NurbSurface                  },
		{ "NurbVersion"                  , NodeName::NurbVersion                  },
		{ "ObjectMetaData"               , NodeName::ObjectMetaData               },
		{ "Objects"                      , NodeName::Objects                      },
		{ "ObjectType"                   , NodeName::ObjectType                   },
		{ "Offset"                       , NodeName::Offset                       },
		{ "OldSection_HierarchyView"     , NodeName::OldSection_HierarchyView     },
		{ "OldSection_VersionFive"       , NodeName::OldSection_VersionFive       },
		{ "OpticalCenterXProperty"       , NodeName::OpticalCenterXProperty       },
		{ "OpticalCenterYProperty"       , NodeName::OpticalCenterYProperty       },
		{ "Order"                        , NodeName::Order                        },
		{ "Original"                     , NodeName::Original                     },
		{ "OriginalFilename"             , NodeName::OriginalFilename             },
		{ "OriginalFormat"               , NodeName::OriginalFormat               },
		{ "Over"                         , NodeName::Over                         },
		{ "Overlay"                      , NodeName::Overlay                      },
		{ "OverSampling"                 , NodeName::OverSampling                 },
		{ "P"                            , NodeName::P                            },
		{ "Package"                      , NodeName::Package                      },
		{ "PAL"                          , NodeName::PAL                          },
		{ "Param2a"                      , NodeName::Param2a                      },
		{ "PasswordProtection"           , NodeName::PasswordProtection           },
		{ "Patch"                        , NodeName::Patch                        },
		{ "PatchType"                    , NodeName::PatchType                    },
		{ "PatchVersion"                 , NodeName::PatchVersion                 },
		{ "Pinlight"                     , NodeName::Pinlight                     },
		{ "PivotEnabled"                 , NodeName::PivotEnabled                 },
		{ "Pivots"                       , NodeName::Pivots                       },
		{ "PixelRatio"                   , NodeName::PixelRatio                   },
		{ "Plane"                        , NodeName::Plane                        },
		{ "PluginParameters"             , NodeName::PluginParameters             },
		{ "Points"                       , NodeName::Points                       },
		{ "PointsIndex"                  , NodeName::PointsIndex                  },
		{ "PolygonGroup"                 , NodeName::PolygonGroup                 },
		{ "PolygonIndexArray"            , NodeName::PolygonIndexArray            },
		{ "PolygonVertexIndex"           , NodeName::PolygonVertexIndex           },
		{ "Pose"                         , NodeName::Pose                         },
		{ "Position"                     , NodeName::Position                     },
		{ "PostRotation"                 , NodeName::PostRotation                 },
		{ "PostTargetRotation"           , NodeName::PostTargetRotation           },
		{ "PrecompFileContent"           , NodeName::PrecompFileContent           },
		{ "PrecompFileName"              , NodeName::PrecompFileName              },
		{ "PreRotation"                  , NodeName::PreRotation                  },
		{ "PreserveBorders"              , NodeName::PreserveBorders              },
		{ "PreserveHardEdges"            , NodeName::PreserveHardEdges            },
		{ "PreviewDivisionLevels"        , NodeName::PreviewDivisionLevels        },
		{ "ProceduralTexture"            , NodeName::ProceduralTexture            },
		{ "PropagateEdgeHardness"        , NodeName::PropagateEdgeHardness        },
		{ "Properties"                   , NodeName::Properties                   },
		{ "Properties70"                 , NodeName::Properties70                 },
		{ "PropertyTemplate"             , NodeName::PropertyTemplate             },
		{ "R"                            , NodeName::R                            },
		{ "Rational"                     , NodeName::Rational                     },
		{ "RAuto"                        , NodeName::RAuto                        },
		{ "RClampType"                   , NodeName::RClampType                   },
		{ "Reference"                    , NodeName::Reference                    },
		{ "ReferenceInformationType"     , NodeName::ReferenceInformationType     },
		{ "References"                   , NodeName::References                   },
		{ "ReferenceTime"                , NodeName::ReferenceTime                },
		{ "ReferenceTimeIndex"           , NodeName::ReferenceTimeIndex           },
		{ "ReferenceTo"                  , NodeName::ReferenceTo                  },
		{ "Reflectivity"                 , NodeName::Reflectivity                 },
		{ "Relations"                    , NodeName::Relations                    },
		{ "RelativeFilename"             , NodeName::RelativeFilename             },
		{ "RelativePrecompFileName"      , NodeName::RelativePrecompFileName      },
		{ "REnable"                      , NodeName::REnable                      },
		{ "RenderDivisionLevels"         , NodeName::RenderDivisionLevels         },
		{ "RendererSetting"              , NodeName::RendererSetting              },
		{ "RestPose"                     , NodeName::RestPose                     },
		{ "Revision"                     , NodeName::Revision                     },
		{ "Roll"                         , NodeName::Roll                         },
		{ "RootNode"                     , NodeName::RootNode                     },
		{ "Rotation"                     , NodeName::Rotation                     },
		{ "RotationPivot"                , NodeName::RotationPivot                },
		{ "RType"                        , NodeName::RType                        },
		{ "RXAxis"                       , NodeName::RXAxis                       },
		{ "RXDefault"                    , NodeName::RXDefault                    },
		{ "RXMax"                        , NodeName::RXMax                        },
		{ "RXMin"                        , NodeName::RXMin                        },
		{ "RYAxis"                       , NodeName::RYAxis                       },
		{ "RYDefault"                    , NodeName::RYDefault                    },
		{ "RYMax"                        , NodeName::RYMax                        },
		{ "RYMin"                        , NodeName::RYMin                        },
		{ "RZAxis"                       , NodeName::RZAxis                       },
		{ "RZDefault"                    , NodeName::RZDefault                    },
		{ "RZMax"                        , NodeName::RZMax                        },
		{ "RZMin"                        , NodeName::RZMin                        },
		{ "S"                            , NodeName::S                            },
		{ "SafeAreaStyle"                , NodeName::SafeAreaStyle                },
		{ "Saturation"                   , NodeName::Saturation                   },
		{ "SAuto"                        , NodeName::SAuto                        },
		{ "Scaling"                      , NodeName::Scaling                      },
		{ "ScalingPivot"                 , NodeName::ScalingPivot                 },
		{ "Scene"                        , NodeName::Scene                        },
		{ "SceneInfo"                    , NodeName::SceneInfo                    },
		{ "SceneReference"               , NodeName::SceneReference               },
		{ "Second"                       , NodeName::Second                       },
		{ "SelectionNode"                , NodeName::SelectionNode                },
		{ "SEnable"                      , NodeName::SEnable                      },
		{ "Settings"                     , NodeName::Settings                     },
		{ "Shading"                      , NodeName::Shading                      },
		{ "ShadingModel"                 , NodeName::ShadingModel                 },
		{ "ShadowIntensity"              , NodeName::ShadowIntensity              },
		{ "ShadowMap"                    , NodeName::ShadowMap                    },
		{ "ShadowPlanes"                 , NodeName::ShadowPlanes                 },
		{ "Shape"                        , NodeName::Shape                        },
		{ "Shininess"                    , NodeName::Shininess                    },
		{ "ShowAudio"                    , NodeName::ShowAudio                    },
		{ "ShowAzimut"                   , NodeName::ShowAzimut                   },
		{ "ShowGrid"                     , NodeName::ShowGrid                     },
		{ "ShowInfoOnMoving"             , NodeName::ShowInfoOnMoving             },
		{ "ShowLabel"                    , NodeName::ShowLabel                    },
		{ "ShowName"                     , NodeName::ShowName                     },
		{ "ShowTimeCode"                 , NodeName::ShowTimeCode                 },
		{ "Size"                         , NodeName::Size                         },
		{ "SkinningType"                 , NodeName::SkinningType                 },
		{ "Smoothing"                    , NodeName::Smoothing                    },
		{ "Smoothness"                   , NodeName::Smoothness                   },
		{ "SnapOnFrames"                 , NodeName::SnapOnFrames                 },
		{ "Softlight"                    , NodeName::Softlight                    },
		{ "SourceCount"                  , NodeName::SourceCount                  },
		{ "Specular"                     , NodeName::Specular                     },
		{ "SphereReflexionMap"           , NodeName::SphereReflexionMap           },
		{ "SphericalReflexionMap"        , NodeName::SphericalReflexionMap        },
		{ "SqueezeRatio"                 , NodeName::SqueezeRatio                 },
		{ "Standard"                     , NodeName::Standard                     },
		{ "Step"                         , NodeName::Step                         },
		{ "Stereo"                       , NodeName::Stereo                       },
		{ "SubDeformer"                  , NodeName::SubDeformer                  },
		{ "Subdiv"                       , NodeName::Subdiv                       },
		{ "SubdivGeometry"               , NodeName::SubdivGeometry               },
		{ "SubdivisionVersion"           , NodeName::SubdivisionVersion           },
		{ "Subject"                      , NodeName::Subject                      },
		{ "Subtract"                     , NodeName::Subtract                     },
		{ "Summary"                      , NodeName::Summary                      },
		{ "SurfaceDisplay"               , NodeName::SurfaceDisplay               },
		{ "SwapUV"                       , NodeName::SwapUV                       },
		{ "Switcher"                     , NodeName::Switcher                     },
		{ "SXDefault"                    , NodeName::SXDefault                    },
		{ "SXMax"                        , NodeName::SXMax                        },
		{ "SXMin"                        , NodeName::SXMin                        },
		{ "SYDefault"                    , NodeName::SYDefault                    },
		{ "SYMax"                        , NodeName::SYMax                        },
		{ "SYMin"                        , NodeName::SYMin                        },
		{ "SZDefault"                    , NodeName::SZDefault                    },
		{ "SZMax"                        , NodeName::SZMax                        },
		{ "SZMin"                        , NodeName::SZMin                        },
		{ "T"                            , NodeName::T                            },
		{ "Take"                         , NodeName::Take                         },
		{ "Takes"                        , NodeName::Takes                        },
		{ "Tangents"                     , NodeName::Tangents                     },
		{ "TangentsIndex"                , NodeName::TangentsIndex                },
		{ "TangentsW"                    , NodeName::TangentsW                    },
		{ "Target"                       , NodeName::Target                       },
		{ "TAuto"                        , NodeName::TAuto                        },
		{ "Template"                     , NodeName::Template                     },
		{ "TEnable"                      , NodeName::TEnable                      },
		{ "Texture"                      , NodeName::Texture                      },
		{ "Texture_Alpha_Source"         , NodeName::Texture_Alpha_Source         },
		{ "Texture_Mapping_Type"         , NodeName::Texture_Mapping_Type         },
		{ "Texture_Planar_Mapping_Normal", NodeName::Texture_Planar_Mapping_Normal},
		{ "TextureAlpha"                 , NodeName::TextureAlpha                 },
		{ "TextureBlendMode"             , NodeName::TextureBlendMode             },
		{ "TextureId"                    , NodeName::TextureId                    },
		{ "TextureIdCount"               , NodeName::TextureIdCount               },
		{ "TextureMode"                  , NodeName::TextureMode                  },
		{ "TextureName"                  , NodeName::TextureName                  },
		{ "TexturePUV"                   , NodeName::TexturePUV                   },
		{ "TextureType"                  , NodeName::TextureType                  },
		{ "TextureUse"                   , NodeName::TextureUse                   },
		{ "TextureUV"                    , NodeName::TextureUV                    },
		{ "TextureUVVerticeIndex"        , NodeName::TextureUVVerticeIndex        },
		{ "TextureVertexIndex"           , NodeName::TextureVertexIndex           },
		{ "Thumbnail"                    , NodeName::Thumbnail                    },
		{ "Tilling"                      , NodeName::Tilling                      },
		{ "Time"                         , NodeName::Time                         },
		{ "TimeFormat"                   , NodeName::TimeFormat                   },
		{ "TimeLineStartTime"            , NodeName::TimeLineStartTime            },
		{ "TimeLineStopTime"             , NodeName::TimeLineStopTime             },
		{ "TimelineX"                    , NodeName::TimelineX                    },
		{ "TimelineXTrack"               , NodeName::TimelineXTrack               },
		{ "TimeMarker"                   , NodeName::TimeMarker                   },
		{ "TimeMode"                     , NodeName::TimeMode                     },
		{ "Title"                        , NodeName::Title                        },
		{ "ToeInAdjust"                  , NodeName::ToeInAdjust                  },
		{ "Total1"                       , NodeName::Total1                       },
		{ "Transform"                    , NodeName::Transform                    },
		{ "TransformLink"                , NodeName::TransformLink                },
		{ "TransformParent"              , NodeName::TransformParent              },
		{ "Translation"                  , NodeName::Translation                  },
		{ "TranslationOffset"            , NodeName::TranslationOffset            },
		{ "Translucent"                  , NodeName::Translucent                  },
		{ "TrimBoundary"                 , NodeName::TrimBoundary                 },
		{ "TrimmedNurbVersion"           , NodeName::TrimmedNurbVersion           },
		{ "TrimNurbsSurface"             , NodeName::TrimNurbsSurface             },
		{ "TurnTable"                    , NodeName::TurnTable                    },
		{ "TXDefault"                    , NodeName::TXDefault                    },
		{ "TXMax"                        , NodeName::TXMax                        },
		{ "TXMin"                        , NodeName::TXMin                        },
		{ "TYDefault"                    , NodeName::TYDefault                    },
		{ "TYMax"                        , NodeName::TYMax                        },
		{ "TYMin"                        , NodeName::TYMin                        },
		{ "Type"                         , NodeName::Type                         },
		{ "TypedIndex"                   , NodeName::TypedIndex                   },
		{ "TypeFlags"                    , NodeName::TypeFlags                    },
		{ "TZDefault"                    , NodeName::TZDefault                    },
		{ "TZMax"                        , NodeName::TZMax                        },
		{ "TZMin"                        , NodeName::TZMin                        },
		{ "UCapped"                      , NodeName::UCapped                      },
		{ "UClosed"                      , NodeName::UClosed                      },
		{ "UCount"                       , NodeName::UCount                       },
		{ "UExtendedCount"               , NodeName::UExtendedCount               },
		{ "Up"                           , NodeName::Up                           },
		{ "UpTargetRotation"             , NodeName::UpTargetRotation             },
		{ "UpVectorModel"                , NodeName::UpVectorModel                },
		{ "UseFrameColor"                , NodeName::UseFrameColor                },
		{ "UseMipMap"                    , NodeName::UseMipMap                    },
		{ "UseOverSamplingTime"          , NodeName::UseOverSamplingTime          },
		{ "UserData"                     , NodeName::UserData                     },
		{ "UserDataArray"                , NodeName::UserDataArray                },
		{ "UserDataDirectCount"          , NodeName::UserDataDirectCount          },
		{ "UserDataDirectIndices"        , NodeName::UserDataDirectIndices        },
		{ "UserDataId"                   , NodeName::UserDataId                   },
		{ "UserDataIndex"                , NodeName::UserDataIndex                },
		{ "UserDataName"                 , NodeName::UserDataName                 },
		{ "UserDataType"                 , NodeName::UserDataType                 },
		{ "UserProperty"                 , NodeName::UserProperty                 },
		{ "UseShadow"                    , NodeName::UseShadow                    },
		{ "UStep"                        , NodeName::UStep                        },
		{ "UTopCap"                      , NodeName::UTopCap                      },
		{ "UV"                           , NodeName::UV                           },
		{ "UVCount"                      , NodeName::UVCount                      },
		{ "UVId"                         , NodeName::UVId                         },
		{ "UVIdCount"                    , NodeName::UVIdCount                    },
		{ "UVIndex"                      , NodeName::UVIndex                      },
		{ "UVMode"                       , NodeName::UVMode                       },
		{ "UVType"                       , NodeName::UVType                       },
		{ "Value"                        , NodeName::Value                        },
		{ "VCapped"                      , NodeName::VCapped                      },
		{ "VClosed"                      , NodeName::VClosed                      },
		{ "VCount"                       , NodeName::VCount                       },
		{ "Version"                      , NodeName::Version                      },
		{ "Version5"                     , NodeName::Version5                     },
		{ "VertexColorInfo"              , NodeName::VertexColorInfo              },
		{ "VertexCrease"                 , NodeName::VertexCrease                 },
		{ "VertexIndexArray"             , NodeName::VertexIndexArray             },
		{ "Vertices"                     , NodeName::Vertices                     },
		{ "VExtendedCount"               , NodeName::VExtendedCount               },
		{ "Video"                        , NodeName::Video                        },
		{ "VideoClipTexture"             , NodeName::VideoClipTexture             },
		{ "ViewFrustum"                  , NodeName::ViewFrustum                  },
		{ "ViewFrustumPlane"             , NodeName::ViewFrustumPlane             },
		{ "ViewFrustumPlaneDistance"     , NodeName::ViewFrustumPlaneDistance     },
		{ "ViewFrustumPlaneDistanceMode" , NodeName::ViewFrustumPlaneDistanceMode },
		{ "ViewLookAt"                   , NodeName::ViewLookAt                   },
		{ "Visibility"                   , NodeName::Visibility                   },
		{ "VistaVision"                  , NodeName::VistaVision                  },
		{ "Vividlight"                   , NodeName::Vividlight                   },
		{ "VStep"                        , NodeName::VStep                        },
		{ "VTopCap"                      , NodeName::VTopCap                      },
		{ "Weights"                      , NodeName::Weights                      },
		{ "WrapU"                        , NodeName::WrapU                        },
		{ "WrapV"                        , NodeName::WrapV                        },
		{ "Year"                         , NodeName::Year                         },
		{ "ZeroParallax"                 , NodeName::ZeroParallax                 },
	};

	NodeName::NodeName(std::string name) {
		std::unordered_map<std::string, NodeName::NodeNameE>::const_iterator it = node_names_strToEnum.find(name);
		if (it == node_names_strToEnum.end()) {
			m_name = name;
			m_have_enum = false;
		} else {
			m_enum = it->second;
			m_have_enum = true;
		}
	}

	std::string NodeName::toString() const {
		if (!m_have_enum) return m_name;
		switch (m_enum) {
		case NodeName::Actor:                         return "Actor";
		case NodeName::Add:                           return "Add";
		case NodeName::Additive:                      return "Additive";
		case NodeName::Aim:                           return "Aim";
		case NodeName::AllSame:                       return "AllSame";
		case NodeName::Alpha:                         return "Alpha";
		case NodeName::Alphas:                        return "Alphas";
		case NodeName::Ambient:                       return "Ambient";
		case NodeName::AmbientLightColor:             return "AmbientLightColor";
		case NodeName::AmbientRenderSettings:         return "AmbientRenderSettings";
		case NodeName::AnimationCurve:                return "AnimationCurve";
		case NodeName::AnimationCurveNode:            return "AnimationCurveNode";
		case NodeName::AnimationEvaluator:            return "AnimationEvaluator";
		case NodeName::AnimationLayer:                return "AnimationLayer";
		case NodeName::AnimationMode:                 return "AnimationMode";
		case NodeName::AnimationStack:                return "AnimationStack";
		case NodeName::Antialiasing:                  return "Antialiasing";
		case NodeName::Aperture:                      return "Aperture";
		case NodeName::ApertureFormat:                return "ApertureFormat";
		case NodeName::ApertureMode:                  return "ApertureMode";
		case NodeName::AspectH:                       return "AspectH";
		case NodeName::AspectType:                    return "AspectType";
		case NodeName::AspectW:                       return "AspectW";
		case NodeName::AssociateModel:                return "AssociateModel";
		case NodeName::AudioColor:                    return "AudioColor";
		case NodeName::Author:                        return "Author";
		case NodeName::Average:                       return "Average";
		case NodeName::AxisLen:                       return "AxisLen";
		case NodeName::BackGroundColor:               return "BackGroundColor";
		case NodeName::BackgroundMode:                return "BackgroundMode";
		case NodeName::BackgroundTexture:             return "BackgroundTexture";
		case NodeName::BackgroundTreshold:            return "BackgroundTreshold";
		case NodeName::BindingOperator:               return "BindingOperator";
		case NodeName::BindingTable:                  return "BindingTable";
		case NodeName::BindPose:                      return "BindPose";
		case NodeName::Binormals:                     return "Binormals";
		case NodeName::BinormalsIndex:                return "BinormalsIndex";
		case NodeName::BinormalsW:                    return "BinormalsW";
		case NodeName::Blend:                         return "Blend";
		case NodeName::BlendMode:                     return "BlendMode";
		case NodeName::BlendModes:                    return "BlendModes";
		case NodeName::BlendShape:                    return "BlendShape";
		case NodeName::BlendShapeChannel:             return "BlendShapeChannel";
		case NodeName::BlendWeights:                  return "BlendWeights";
		case NodeName::Boundary:                      return "Boundary";
		case NodeName::BoundaryEdge:                  return "BoundaryEdge";
		case NodeName::BoundaryRule:                  return "BoundaryRule";
		case NodeName::BoundaryVersion:               return "BoundaryVersion";
		case NodeName::BumpNormalMap:                 return "BumpNormalMap";
		case NodeName::ByEdge:                        return "ByEdge";
		case NodeName::ByFace:                        return "ByFace";
		case NodeName::ByModel:                       return "ByModel";
		case NodeName::ByPolygon:                     return "ByPolygon";
		case NodeName::ByPolygonVertex:               return "ByPolygonVertex";
		case NodeName::ByVertice:                     return "ByVertice";
		case NodeName::C:                             return "C";
		case NodeName::Cache:                         return "Cache";
		case NodeName::CacheChannel:                  return "CacheChannel";
		case NodeName::CachePath:                     return "CachePath";
		case NodeName::Camera:                        return "Camera";
		case NodeName::CameraAndLens:                 return "CameraAndLens";
		case NodeName::CameraAperture:                return "CameraAperture";
		case NodeName::CameraId:                      return "CameraId";
		case NodeName::CameraIndexName:               return "CameraIndexName";
		case NodeName::CameraLock:                    return "CameraLock";
		case NodeName::CameraName:                    return "CameraName";
		case NodeName::CameraOrthoZoom:               return "CameraOrthoZoom";
		case NodeName::CastLight:                     return "CastLight";
		case NodeName::Character:                     return "Character";
		case NodeName::Children:                      return "Children";
		case NodeName::Clip:                          return "Clip";
		case NodeName::Closed:                        return "Closed";
		case NodeName::Collection:                    return "Collection";
		case NodeName::CollectionExclusive:           return "CollectionExclusive";
		case NodeName::Color:                         return "Color";
		case NodeName::Colorburn:                     return "Colorburn";
		case NodeName::Colordodge:                    return "Colordodge";
		case NodeName::ColorIndex:                    return "ColorIndex";
		case NodeName::Colors:                        return "Colors";
		case NodeName::Comment:                       return "Comment";
		case NodeName::Comments:                      return "Comments";
		case NodeName::Component:                     return "Component";
		case NodeName::Connections:                   return "Connections";
		case NodeName::Constraint:                    return "Constraint";
		case NodeName::Consumer:                      return "Consumer";
		case NodeName::Consumers:                     return "Consumers";
		case NodeName::Container:                     return "Container";
		case NodeName::Content:                       return "Content";
		case NodeName::ContentCount:                  return "ContentCount";
		case NodeName::ControlSetPlug:                return "ControlSetPlug";
		case NodeName::Count:                         return "Count";
		case NodeName::CreationTime:                  return "CreationTime";
		case NodeName::CreationTimeStamp:             return "CreationTimeStamp";
		case NodeName::Creator:                       return "Creator";
		case NodeName::Cropping:                      return "Cropping";
		case NodeName::Culling:                       return "Culling";
		case NodeName::CullingOff:                    return "CullingOff";
		case NodeName::CullingOnCCW:                  return "CullingOnCCW";
		case NodeName::CullingOnCW:                   return "CullingOnCW";
		case NodeName::Current:                       return "Current";
		case NodeName::CurrentLevel:                  return "CurrentLevel";
		case NodeName::Custom:                        return "Custom";
		case NodeName::Darken:                        return "Darken";
		case NodeName::Darkercolor:                   return "Darkercolor";
		case NodeName::Day:                           return "Day";
		case NodeName::Default:                       return "Default";
		case NodeName::DefaultCamera:                 return "DefaultCamera";
		case NodeName::DefaultViewingMode:            return "DefaultViewingMode";
		case NodeName::Definitions:                   return "Definitions";
		case NodeName::Deformer:                      return "Deformer";
		case NodeName::DeformPercent:                 return "DeformPercent";
		case NodeName::DepthOfField:                  return "DepthOfField";
		case NodeName::DestinationCount:              return "DestinationCount";
		case NodeName::Device:                        return "Device";
		case NodeName::Difference:                    return "Difference";
		case NodeName::Diffuse:                       return "Diffuse";
		case NodeName::Dimension:                     return "Dimension";
		case NodeName::Dimensions:                    return "Dimensions";
		case NodeName::Direct:                        return "Direct";
		case NodeName::DisplayLayer:                  return "DisplayLayer";
		case NodeName::DisplayMode:                   return "DisplayMode";
		case NodeName::DisplaySafeArea:               return "DisplaySafeArea";
		case NodeName::DisplaySafeAreaOnRender:       return "DisplaySafeAreaOnRender";
		case NodeName::DisplaySubdivisions:           return "DisplaySubdivisions";
		case NodeName::Dissolve:                      return "Dissolve";
		case NodeName::Divide:                        return "Divide";
		case NodeName::Doc:                           return "Doc";
		case NodeName::Document:                      return "Document";
		case NodeName::Documents:                     return "Documents";
		case NodeName::DrawComponent:                 return "DrawComponent";
		case NodeName::DualQuaternion:                return "DualQuaternion";
		case NodeName::Dynavision:                    return "Dynavision";
		case NodeName::EdgeCrease:                    return "EdgeCrease";
		case NodeName::EdgeIndexArray:                return "EdgeIndexArray";
		case NodeName::Edges:                         return "Edges";
		case NodeName::Embedding:                     return "Embedding";
		case NodeName::Emissive:                      return "Emissive";
		case NodeName::EncryptionType:                return "EncryptionType";
		case NodeName::Entry:                         return "Entry";
		case NodeName::Exclusion:                     return "Exclusion";
		case NodeName::FarPlane:                      return "FarPlane";
		case NodeName::FBXHeaderExtension:            return "FBXHeaderExtension";
		case NodeName::FBXHeaderVersion:              return "FBXHeaderVersion";
		case NodeName::FBXVersion:                    return "FBXVersion";
		case NodeName::FieldOfViewXProperty:          return "FieldOfViewXProperty";
		case NodeName::FieldOfViewYProperty:          return "FieldOfViewYProperty";
		case NodeName::File:                          return "File";
		case NodeName::FileId:                        return "FileId";
		case NodeName::Filename:                      return "Filename";
		case NodeName::FileName:                      return "FileName";
		case NodeName::FilePathUrl:                   return "FilePathUrl";
		case NodeName::FilmOffsetLeftCam:             return "FilmOffsetLeftCam";
		case NodeName::FilmOffsetRightCam:            return "FilmOffsetRightCam";
		case NodeName::FlipNormals:                   return "FlipNormals";
		case NodeName::FlogEnable:                    return "FlogEnable";
		case NodeName::FocalLength:                   return "FocalLength";
		case NodeName::FogColor:                      return "FogColor";
		case NodeName::FogDensity:                    return "FogDensity";
		case NodeName::FogEnd:                        return "FogEnd";
		case NodeName::FogMode:                       return "FogMode";
		case NodeName::FogOptions:                    return "FogOptions";
		case NodeName::FogStart:                      return "FogStart";
		case NodeName::Folder:                        return "Folder";
		case NodeName::ForegroundTransparent:         return "ForegroundTransparent";
		case NodeName::Form:                          return "Form";
		case NodeName::Format:                        return "Format";
		case NodeName::FormatName:                    return "FormatName";
		case NodeName::FrameColor:                    return "FrameColor";
		case NodeName::FrameRate:                     return "FrameRate";
		case NodeName::FullWeights:                   return "FullWeights";
		case NodeName::GenericNode:                   return "GenericNode";
		case NodeName::Geometry:                      return "Geometry";
		case NodeName::GeometryLayer:                 return "GeometryLayer";
		case NodeName::GeometryUVInfo:                return "GeometryUVInfo";
		case NodeName::GeometryVersion:               return "GeometryVersion";
		case NodeName::GeometryWeightedMap:           return "GeometryWeightedMap";
		case NodeName::GlobalSettings:                return "GlobalSettings";
		case NodeName::Gobo:                          return "Gobo";
		case NodeName::GoboManager:                   return "GoboManager";
		case NodeName::GoboName:                      return "GoboName";
		case NodeName::GoboPath:                      return "GoboPath";
		case NodeName::Hardmix:                       return "Hardmix";
		case NodeName::HD:                            return "HD";
		case NodeName::Hidden:                        return "Hidden";
		case NodeName::HierarchyView:                 return "HierarchyView";
		case NodeName::Hole:                          return "Hole";
		case NodeName::Hour:                          return "Hour";
		case NodeName::Hue:                           return "Hue";
		case NodeName::IKPivot:                       return "IKPivot";
		case NodeName::IKPull:                        return "IKPull";
		case NodeName::IKPullHips:                    return "IKPullHips";
		case NodeName::IKReachRotation:               return "IKReachRotation";
		case NodeName::IKReachTranslation:            return "IKReachTranslation";
		case NodeName::ImageData:                     return "ImageData";
		case NodeName::ImageEncoding:                 return "ImageEncoding";
		case NodeName::Imax:                          return "Imax";
		case NodeName::Implementation:                return "Implementation";
		case NodeName::Index:                         return "Index";
		case NodeName::Indexes:                       return "Indexes";
		case NodeName::IndexMapping:                  return "IndexMapping";
		case NodeName::IndexToDirect:                 return "IndexToDirect";
		case NodeName::InteraxialSeparation:          return "InteraxialSeparation";
		case NodeName::InternalEdges:                 return "InternalEdges";
		case NodeName::Keywords:                      return "Keywords";
		case NodeName::KFbxEnvironment:               return "KFbxEnvironment";
		case NodeName::KnotVector:                    return "KnotVector";
		case NodeName::KnotVectorU:                   return "KnotVectorU";
		case NodeName::KnotVectorV:                   return "KnotVectorV";
		case NodeName::Label:                         return "Label";
		case NodeName::Layer:                         return "Layer";
		case NodeName::LayeredTexture:                return "LayeredTexture";
		case NodeName::LayerElement:                  return "LayerElement";
		case NodeName::LayerElementBinormal:          return "LayerElementBinormal";
		case NodeName::LayerElementColor:             return "LayerElementColor";
		case NodeName::LayerElementEdgeCrease:        return "LayerElementEdgeCrease";
		case NodeName::LayerElementHole:              return "LayerElementHole";
		case NodeName::LayerElementMaterial:          return "LayerElementMaterial";
		case NodeName::LayerElementNormal:            return "LayerElementNormal";
		case NodeName::LayerElementPolygonGroup:      return "LayerElementPolygonGroup";
		case NodeName::LayerElementSmoothing:         return "LayerElementSmoothing";
		case NodeName::LayerElementTangent:           return "LayerElementTangent";
		case NodeName::LayerElementTexture:           return "LayerElementTexture";
		case NodeName::LayerElementUserData:          return "LayerElementUserData";
		case NodeName::LayerElementUV:                return "LayerElementUV";
		case NodeName::LayerElementVertexCrease:      return "LayerElementVertexCrease";
		case NodeName::LayerElementVisibility:        return "LayerElementVisibility";
		case NodeName::LayerTextureInfo:              return "LayerTextureInfo";
		case NodeName::LevelCount:                    return "LevelCount";
		case NodeName::Library:                       return "Library";
		case NodeName::Lighten:                       return "Lighten";
		case NodeName::Lightercolor:                  return "Lightercolor";
		case NodeName::LightGobo:                     return "LightGobo";
		case NodeName::LightMap:                      return "LightMap";
		case NodeName::LightType:                     return "LightType";
		case NodeName::LightTypeVersion:              return "LightTypeVersion";
		case NodeName::LimbLength:                    return "LimbLength";
		case NodeName::Limits:                        return "Limits";
		case NodeName::Line:                          return "Line";
		case NodeName::Linear:                        return "Linear";
		case NodeName::Linearburn:                    return "Linearburn";
		case NodeName::Lineardodge:                   return "Lineardodge";
		case NodeName::Linearlight:                   return "Linearlight";
		case NodeName::LineVersion:                   return "LineVersion";
		case NodeName::Link:                          return "Link";
		case NodeName::Link_DeformAcuracy:            return "Link_DeformAcuracy";
		case NodeName::LocalTime:                     return "LocalTime";
		case NodeName::Look:                          return "Look";
		case NodeName::LookAt:                        return "LookAt";
		case NodeName::LookAtModel:                   return "LookAtModel";
		case NodeName::Loop:                          return "Loop";
		case NodeName::Luminosity:                    return "Luminosity";
		case NodeName::MappingInformationType:        return "MappingInformationType";
		case NodeName::MarkerSet:                     return "MarkerSet";
		case NodeName::Material:                      return "Material";
		case NodeName::MaterialAssignation:           return "MaterialAssignation";
		case NodeName::MaterialMode:                  return "MaterialMode";
		case NodeName::Materials:                     return "Materials";
		case NodeName::Max:                           return "Max";
		case NodeName::MaxBlend:                      return "MaxBlend";
		case NodeName::Media:                         return "Media";
		case NodeName::Mesh:                          return "Mesh";
		case NodeName::MetaData:                      return "MetaData";
		case NodeName::Millisecond:                   return "Millisecond";
		case NodeName::Min:                           return "Min";
		case NodeName::Minute:                        return "Minute";
		case NodeName::Mode:                          return "Mode";
		case NodeName::Model:                         return "Model";
		case NodeName::ModelUVScaling:                return "ModelUVScaling";
		case NodeName::ModelUVTranslation:            return "ModelUVTranslation";
		case NodeName::Modulate:                      return "Modulate";
		case NodeName::Modulate2:                     return "Modulate2";
		case NodeName::Month:                         return "Month";
		case NodeName::MultiLayer:                    return "MultiLayer";
		case NodeName::MultiplicityU:                 return "MultiplicityU";
		case NodeName::MultiplicityV:                 return "MultiplicityV";
		case NodeName::Name:                          return "Name";
		case NodeName::NearPlane:                     return "NearPlane";
		case NodeName::NodeAttribute:                 return "NodeAttribute";
		case NodeName::NodeAttributeName:             return "NodeAttributeName";
		case NodeName::NodeAttributeRefTo:            return "NodeAttributeRefTo";
		case NodeName::NodeId:                        return "NodeId";
		case NodeName::NoMappingInformation:          return "NoMappingInformation";
		case NodeName::Normal:                        return "Normal";
		case NodeName::Normals:                       return "Normals";
		case NodeName::NormalsIndex:                  return "NormalsIndex";
		case NodeName::NormalsW:                      return "NormalsW";
		case NodeName::NTSC:                          return "NTSC";
		case NodeName::Nurb:                          return "Nurb";
		case NodeName::NurbOrder:                     return "NurbOrder";
		case NodeName::NurbsCurve:                    return "NurbsCurve";
		case NodeName::NurbsCurveVersion:             return "NurbsCurveVersion";
		case NodeName::NurbsSurface:                  return "NurbsSurface";
		case NodeName::NurbsSurfaceOrder:             return "NurbsSurfaceOrder";
		case NodeName::NurbsSurfaceVersion:           return "NurbsSurfaceVersion";
		case NodeName::NurbSurface:                   return "NurbSurface";
		case NodeName::NurbVersion:                   return "NurbVersion";
		case NodeName::ObjectMetaData:                return "ObjectMetaData";
		case NodeName::Objects:                       return "Objects";
		case NodeName::ObjectType:                    return "ObjectType";
		case NodeName::Offset:                        return "Offset";
		case NodeName::OldSection_HierarchyView:      return "OldSection_HierarchyView";
		case NodeName::OldSection_VersionFive:        return "OldSection_VersionFive";
		case NodeName::OpticalCenterXProperty:        return "OpticalCenterXProperty";
		case NodeName::OpticalCenterYProperty:        return "OpticalCenterYProperty";
		case NodeName::Order:                         return "Order";
		case NodeName::Original:                      return "Original";
		case NodeName::OriginalFilename:              return "OriginalFilename";
		case NodeName::OriginalFormat:                return "OriginalFormat";
		case NodeName::Over:                          return "Over";
		case NodeName::Overlay:                       return "Overlay";
		case NodeName::OverSampling:                  return "OverSampling";
		case NodeName::P:                             return "P";
		case NodeName::Package:                       return "Package";
		case NodeName::PAL:                           return "PAL";
		case NodeName::Param2a:                       return "Param2a";
		case NodeName::PasswordProtection:            return "PasswordProtection";
		case NodeName::Patch:                         return "Patch";
		case NodeName::PatchType:                     return "PatchType";
		case NodeName::PatchVersion:                  return "PatchVersion";
		case NodeName::Pinlight:                      return "Pinlight";
		case NodeName::PivotEnabled:                  return "PivotEnabled";
		case NodeName::Pivots:                        return "Pivots";
		case NodeName::PixelRatio:                    return "PixelRatio";
		case NodeName::Plane:                         return "Plane";
		case NodeName::PluginParameters:              return "PluginParameters";
		case NodeName::Points:                        return "Points";
		case NodeName::PointsIndex:                   return "PointsIndex";
		case NodeName::PolygonGroup:                  return "PolygonGroup";
		case NodeName::PolygonIndexArray:             return "PolygonIndexArray";
		case NodeName::PolygonVertexIndex:            return "PolygonVertexIndex";
		case NodeName::Pose:                          return "Pose";
		case NodeName::Position:                      return "Position";
		case NodeName::PostRotation:                  return "PostRotation";
		case NodeName::PostTargetRotation:            return "PostTargetRotation";
		case NodeName::PrecompFileContent:            return "PrecompFileContent";
		case NodeName::PrecompFileName:               return "PrecompFileName";
		case NodeName::PreRotation:                   return "PreRotation";
		case NodeName::PreserveBorders:               return "PreserveBorders";
		case NodeName::PreserveHardEdges:             return "PreserveHardEdges";
		case NodeName::PreviewDivisionLevels:         return "PreviewDivisionLevels";
		case NodeName::ProceduralTexture:             return "ProceduralTexture";
		case NodeName::PropagateEdgeHardness:         return "PropagateEdgeHardness";
		case NodeName::Properties:                    return "Properties";
		case NodeName::Properties70:                  return "Properties70";
		case NodeName::PropertyTemplate:              return "PropertyTemplate";
		case NodeName::R:                             return "R";
		case NodeName::Rational:                      return "Rational";
		case NodeName::RAuto:                         return "RAuto";
		case NodeName::RClampType:                    return "RClampType";
		case NodeName::Reference:                     return "Reference";
		case NodeName::ReferenceInformationType:      return "ReferenceInformationType";
		case NodeName::References:                    return "References";
		case NodeName::ReferenceTime:                 return "ReferenceTime";
		case NodeName::ReferenceTimeIndex:            return "ReferenceTimeIndex";
		case NodeName::ReferenceTo:                   return "ReferenceTo";
		case NodeName::Reflectivity:                  return "Reflectivity";
		case NodeName::Relations:                     return "Relations";
		case NodeName::RelativeFilename:              return "RelativeFilename";
		case NodeName::RelativePrecompFileName:       return "RelativePrecompFileName";
		case NodeName::REnable:                       return "REnable";
		case NodeName::RenderDivisionLevels:          return "RenderDivisionLevels";
		case NodeName::RendererSetting:               return "RendererSetting";
		case NodeName::RestPose:                      return "RestPose";
		case NodeName::Revision:                      return "Revision";
		case NodeName::Roll:                          return "Roll";
		case NodeName::RootNode:                      return "RootNode";
		case NodeName::Rotation:                      return "Rotation";
		case NodeName::RotationPivot:                 return "RotationPivot";
		case NodeName::RType:                         return "RType";
		case NodeName::RXAxis:                        return "RXAxis";
		case NodeName::RXDefault:                     return "RXDefault";
		case NodeName::RXMax:                         return "RXMax";
		case NodeName::RXMin:                         return "RXMin";
		case NodeName::RYAxis:                        return "RYAxis";
		case NodeName::RYDefault:                     return "RYDefault";
		case NodeName::RYMax:                         return "RYMax";
		case NodeName::RYMin:                         return "RYMin";
		case NodeName::RZAxis:                        return "RZAxis";
		case NodeName::RZDefault:                     return "RZDefault";
		case NodeName::RZMax:                         return "RZMax";
		case NodeName::RZMin:                         return "RZMin";
		case NodeName::S:                             return "S";
		case NodeName::SafeAreaStyle:                 return "SafeAreaStyle";
		case NodeName::Saturation:                    return "Saturation";
		case NodeName::SAuto:                         return "SAuto";
		case NodeName::Scaling:                       return "Scaling";
		case NodeName::ScalingPivot:                  return "ScalingPivot";
		case NodeName::Scene:                         return "Scene";
		case NodeName::SceneInfo:                     return "SceneInfo";
		case NodeName::SceneReference:                return "SceneReference";
		case NodeName::Second:                        return "Second";
		case NodeName::SelectionNode:                 return "SelectionNode";
		case NodeName::SEnable:                       return "SEnable";
		case NodeName::Settings:                      return "Settings";
		case NodeName::Shading:                       return "Shading";
		case NodeName::ShadingModel:                  return "ShadingModel";
		case NodeName::ShadowIntensity:               return "ShadowIntensity";
		case NodeName::ShadowMap:                     return "ShadowMap";
		case NodeName::ShadowPlanes:                  return "ShadowPlanes";
		case NodeName::Shape:                         return "Shape";
		case NodeName::Shininess:                     return "Shininess";
		case NodeName::ShowAudio:                     return "ShowAudio";
		case NodeName::ShowAzimut:                    return "ShowAzimut";
		case NodeName::ShowGrid:                      return "ShowGrid";
		case NodeName::ShowInfoOnMoving:              return "ShowInfoOnMoving";
		case NodeName::ShowLabel:                     return "ShowLabel";
		case NodeName::ShowName:                      return "ShowName";
		case NodeName::ShowTimeCode:                  return "ShowTimeCode";
		case NodeName::Size:                          return "Size";
		case NodeName::SkinningType:                  return "SkinningType";
		case NodeName::Smoothing:                     return "Smoothing";
		case NodeName::Smoothness:                    return "Smoothness";
		case NodeName::SnapOnFrames:                  return "SnapOnFrames";
		case NodeName::Softlight:                     return "Softlight";
		case NodeName::SourceCount:                   return "SourceCount";
		case NodeName::Specular:                      return "Specular";
		case NodeName::SphereReflexionMap:            return "SphereReflexionMap";
		case NodeName::SphericalReflexionMap:         return "SphericalReflexionMap";
		case NodeName::SqueezeRatio:                  return "SqueezeRatio";
		case NodeName::Standard:                      return "Standard";
		case NodeName::Step:                          return "Step";
		case NodeName::Stereo:                        return "Stereo";
		case NodeName::SubDeformer:                   return "SubDeformer";
		case NodeName::Subdiv:                        return "Subdiv";
		case NodeName::SubdivGeometry:                return "SubdivGeometry";
		case NodeName::SubdivisionVersion:            return "SubdivisionVersion";
		case NodeName::Subject:                       return "Subject";
		case NodeName::Subtract:                      return "Subtract";
		case NodeName::Summary:                       return "Summary";
		case NodeName::SurfaceDisplay:                return "SurfaceDisplay";
		case NodeName::SwapUV:                        return "SwapUV";
		case NodeName::Switcher:                      return "Switcher";
		case NodeName::SXDefault:                     return "SXDefault";
		case NodeName::SXMax:                         return "SXMax";
		case NodeName::SXMin:                         return "SXMin";
		case NodeName::SYDefault:                     return "SYDefault";
		case NodeName::SYMax:                         return "SYMax";
		case NodeName::SYMin:                         return "SYMin";
		case NodeName::SZDefault:                     return "SZDefault";
		case NodeName::SZMax:                         return "SZMax";
		case NodeName::SZMin:                         return "SZMin";
		case NodeName::T:                             return "T";
		case NodeName::Take:                          return "Take";
		case NodeName::Takes:                         return "Takes";
		case NodeName::Tangents:                      return "Tangents";
		case NodeName::TangentsIndex:                 return "TangentsIndex";
		case NodeName::TangentsW:                     return "TangentsW";
		case NodeName::Target:                        return "Target";
		case NodeName::TAuto:                         return "TAuto";
		case NodeName::Template:                      return "Template";
		case NodeName::TEnable:                       return "TEnable";
		case NodeName::Texture:                       return "Texture";
		case NodeName::Texture_Alpha_Source:          return "Texture_Alpha_Source";
		case NodeName::Texture_Mapping_Type:          return "Texture_Mapping_Type";
		case NodeName::Texture_Planar_Mapping_Normal: return "Texture_Planar_Mapping_Normal";
		case NodeName::TextureAlpha:                  return "TextureAlpha";
		case NodeName::TextureBlendMode:              return "TextureBlendMode";
		case NodeName::TextureId:                     return "TextureId";
		case NodeName::TextureIdCount:                return "TextureIdCount";
		case NodeName::TextureMode:                   return "TextureMode";
		case NodeName::TextureName:                   return "TextureName";
		case NodeName::TexturePUV:                    return "TexturePUV";
		case NodeName::TextureType:                   return "TextureType";
		case NodeName::TextureUse:                    return "TextureUse";
		case NodeName::TextureUV:                     return "TextureUV";
		case NodeName::TextureUVVerticeIndex:         return "TextureUVVerticeIndex";
		case NodeName::TextureVertexIndex:            return "TextureVertexIndex";
		case NodeName::Thumbnail:                     return "Thumbnail";
		case NodeName::Tilling:                       return "Tilling";
		case NodeName::Time:                          return "Time";
		case NodeName::TimeFormat:                    return "TimeFormat";
		case NodeName::TimeLineStartTime:             return "TimeLineStartTime";
		case NodeName::TimeLineStopTime:              return "TimeLineStopTime";
		case NodeName::TimelineX:                     return "TimelineX";
		case NodeName::TimelineXTrack:                return "TimelineXTrack";
		case NodeName::TimeMarker:                    return "TimeMarker";
		case NodeName::TimeMode:                      return "TimeMode";
		case NodeName::Title:                         return "Title";
		case NodeName::ToeInAdjust:                   return "ToeInAdjust";
		case NodeName::Total1:                        return "Total1";
		case NodeName::Transform:                     return "Transform";
		case NodeName::TransformLink:                 return "TransformLink";
		case NodeName::TransformParent:               return "TransformParent";
		case NodeName::Translation:                   return "Translation";
		case NodeName::TranslationOffset:             return "TranslationOffset";
		case NodeName::Translucent:                   return "Translucent";
		case NodeName::TrimBoundary:                  return "TrimBoundary";
		case NodeName::TrimmedNurbVersion:            return "TrimmedNurbVersion";
		case NodeName::TrimNurbsSurface:              return "TrimNurbsSurface";
		case NodeName::TurnTable:                     return "TurnTable";
		case NodeName::TXDefault:                     return "TXDefault";
		case NodeName::TXMax:                         return "TXMax";
		case NodeName::TXMin:                         return "TXMin";
		case NodeName::TYDefault:                     return "TYDefault";
		case NodeName::TYMax:                         return "TYMax";
		case NodeName::TYMin:                         return "TYMin";
		case NodeName::Type:                          return "Type";
		case NodeName::TypedIndex:                    return "TypedIndex";
		case NodeName::TypeFlags:                     return "TypeFlags";
		case NodeName::TZDefault:                     return "TZDefault";
		case NodeName::TZMax:                         return "TZMax";
		case NodeName::TZMin:                         return "TZMin";
		case NodeName::UCapped:                       return "UCapped";
		case NodeName::UClosed:                       return "UClosed";
		case NodeName::UCount:                        return "UCount";
		case NodeName::UExtendedCount:                return "UExtendedCount";
		case NodeName::Up:                            return "Up";
		case NodeName::UpTargetRotation:              return "UpTargetRotation";
		case NodeName::UpVectorModel:                 return "UpVectorModel";
		case NodeName::UseFrameColor:                 return "UseFrameColor";
		case NodeName::UseMipMap:                     return "UseMipMap";
		case NodeName::UseOverSamplingTime:           return "UseOverSamplingTime";
		case NodeName::UserData:                      return "UserData";
		case NodeName::UserDataArray:                 return "UserDataArray";
		case NodeName::UserDataDirectCount:           return "UserDataDirectCount";
		case NodeName::UserDataDirectIndices:         return "UserDataDirectIndices";
		case NodeName::UserDataId:                    return "UserDataId";
		case NodeName::UserDataIndex:                 return "UserDataIndex";
		case NodeName::UserDataName:                  return "UserDataName";
		case NodeName::UserDataType:                  return "UserDataType";
		case NodeName::UserProperty:                  return "UserProperty";
		case NodeName::UseShadow:                     return "UseShadow";
		case NodeName::UStep:                         return "UStep";
		case NodeName::UTopCap:                       return "UTopCap";
		case NodeName::UV:                            return "UV";
		case NodeName::UVCount:                       return "UVCount";
		case NodeName::UVId:                          return "UVId";
		case NodeName::UVIdCount:                     return "UVIdCount";
		case NodeName::UVIndex:                       return "UVIndex";
		case NodeName::UVMode:                        return "UVMode";
		case NodeName::UVType:                        return "UVType";
		case NodeName::Value:                         return "Value";
		case NodeName::VCapped:                       return "VCapped";
		case NodeName::VClosed:                       return "VClosed";
		case NodeName::VCount:                        return "VCount";
		case NodeName::Version:                       return "Version";
		case NodeName::Version5:                      return "Version5";
		case NodeName::VertexColorInfo:               return "VertexColorInfo";
		case NodeName::VertexCrease:                  return "VertexCrease";
		case NodeName::VertexIndexArray:              return "VertexIndexArray";
		case NodeName::Vertices:                      return "Vertices";
		case NodeName::VExtendedCount:                return "VExtendedCount";
		case NodeName::Video:                         return "Video";
		case NodeName::VideoClipTexture:              return "VideoClipTexture";
		case NodeName::ViewFrustum:                   return "ViewFrustum";
		case NodeName::ViewFrustumPlane:              return "ViewFrustumPlane";
		case NodeName::ViewFrustumPlaneDistance:      return "ViewFrustumPlaneDistance";
		case NodeName::ViewFrustumPlaneDistanceMode:  return "ViewFrustumPlaneDistanceMode";
		case NodeName::ViewLookAt:                    return "ViewLookAt";
		case NodeName::Visibility:                    return "Visibility";
		case NodeName::VistaVision:                   return "VistaVision";
		case NodeName::Vividlight:                    return "Vividlight";
		case NodeName::VStep:                         return "VStep";
		case NodeName::VTopCap:                       return "VTopCap";
		case NodeName::Weights:                       return "Weights";
		case NodeName::WrapU:                         return "WrapU";
		case NodeName::WrapV:                         return "WrapV";
		case NodeName::Year:                          return "Year";
		case NodeName::ZeroParallax:                  return "ZeroParallax";
		}
		return "";
	}

	std::ostream& operator<<(std::ostream& os, const NodeName &nodeName) {
		return os << nodeName.toString();
	}
}